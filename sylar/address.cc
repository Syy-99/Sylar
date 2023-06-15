#include "address.h"
#include "log.h"
#include <sstream>
#include <netdb.h>
#include <ifaddrs.h>
#include <stddef.h>

#include "endian.h"
namespace sylar {

static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

/// 创建掩码
template<class T>
static T CreateMask(uint32_t bits) {
    return (1 << (sizeof(T)*8 - bits)) - 1;
} 

/// 统计掩码长度， value通常是个IP地址
/// uint32_t只能记录32位 
/*
 * 假设value=12 -> 1110, 那么子网掩码长度应该是3，计算逻辑如下：
 * 1110 & 1101 = 1100; 1100 & 1011 = 1000; 1000 & 0111 = 0000;
 * 正好result=3 
 */
template<class T>
static uint32_t CountBytes(T value) {
    uint32_t result = 0;
    for(; value; ++result) {
        value &= value - 1;     // 抹掉最右侧的1
    }
    return result;
}

bool Address::Lookup(std::vector<Address::ptr>& result, const std::string& host,
                     int family, int type, int protocol) {
    addrinfo hints, *results, *next;
    hints.ai_flags = 0;
    hints.ai_family = family;       // 限制
    hints.ai_socktype = type;       // 限制,默认为0，无限制，支持三种套接字类型(一个端口支持三种类型)
    hints.ai_protocol = protocol;   // 限制默认为0，无限制，对应上面三种套接字类型
    hints.ai_addrlen = 0;
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    std::string node;   // ???
    const char* service = NULL; // ip端口
    
    /// ??? 下面几个if逻辑是做什么的???

    //[fe80::5a11:22ff:fea8:a11a]:0 形式
    if(!host.empty() && host[0] == '[') {   //???? host[0] == '[' ?
        // 找到']'的指针
        const char* endipv6 = (const char*)memchr(host.c_str() + 1, ']', host.size() - 1);  
        if(endipv6) {
            //TODO check out of range
            if(*(endipv6 + 1) == ':') { 
                service = endipv6 + 2;  // servcie执行服务的起始位置
            }
            node = host.substr(1, endipv6 - host.c_str() - 1);  // ????
        }
    }

    // www.baidu.com:80
    if(node.empty()) {  // node为空，说明没有经过上面的if处理
        service = (const char*)memchr(host.c_str(), ':', host.size());
        if(service) {
            if(!memchr(service + 1, ':', host.c_str() + host.size() - service - 1)) {
                node = host.substr(0, service - host.c_str());
                ++service;
            }
        }
    }

    // www.baidu.com
    if(node.empty()) {  // 说明没有经过上面的if处理，也就说没有:80
        node = host;
    }

    int error = getaddrinfo(node.c_str(), service, &hints, &results);
    if(error) {
        SYLAR_LOG_DEBUG(g_logger) << "Address::Lookup getaddress(" << host << ", "
            << family << ", " << type << ") err=" << error << " errstr="
            << gai_strerror(error);
        return false;
    }

    next = results;
    while(next) {
        /// 遍历列表
        result.push_back(Create(next->ai_addr, (socklen_t)next->ai_addrlen));
        //SYLAR_LOG_INFO(g_logger) << ((sockaddr_in*)next->ai_addr)->sin_addr.s_addr;
        next = next->ai_next;
    }

    freeaddrinfo(results);
    return !result.empty();
}

Address::ptr Address::LookupAny(const std::string& host,
                                int family, int type, int protocol) {
    std::vector<Address::ptr> result;
    if(Lookup(result, host, family, type, protocol)) {
        return result[0];   // 获得所有地址的第一个
    }
    return nullptr;
}

IPAddress::ptr Address::LookupAnyIPAddress(const std::string& host,
                                int family, int type, int protocol) {
    std::vector<Address::ptr> result;
    if(Lookup(result, host, family, type, protocol)) {
        //for(auto& i : result) {
        //    std::cout << i->toString() << std::endl;
        //}
        for(auto& i : result) {
            IPAddress::ptr v = std::dynamic_pointer_cast<IPAddress>(i);
            if(v) {
                return v;   // 返回第一个
            }
        }
    }
    return nullptr;
}

bool Address::GetInterfaceAddresses(std::multimap<std::string,std::pair<Address::ptr,
                                    uint32_t> >& result,int family) {
    struct ifaddrs *next, *results;
    if(getifaddrs(&results) != 0) { // getifaddrs: 获得网卡地址
        SYLAR_LOG_DEBUG(g_logger) << "Address::GetInterfaceAddresses getifaddrs "
            " err=" << errno << " errstr=" << strerror(errno);
        return false;
    }

    try {
        for(next = results; next; next = next->ifa_next) {
            Address::ptr addr;
            uint32_t prefix_len = ~0u;
            // 不是我们要找的地址类型
            if(family != AF_UNSPEC && family != next->ifa_addr->sa_family) {
                continue;
            }

            switch(next->ifa_addr->sa_family) {
                case AF_INET:
                    {
                        addr = Create(next->ifa_addr, sizeof(sockaddr_in));
                        uint32_t netmask = ((sockaddr_in*)next->ifa_netmask)->sin_addr.s_addr;
                        prefix_len = CountBytes(netmask);
                    }
                    break;
                case AF_INET6:
                    {
                        addr = Create(next->ifa_addr, sizeof(sockaddr_in6));
                        in6_addr& netmask = ((sockaddr_in6*)next->ifa_netmask)->sin6_addr;
                        prefix_len = 0;
                        for(int i = 0; i < 16; ++i) {
                            prefix_len += CountBytes(netmask.s6_addr[i]);
                        }
                    }
                    break;
                default:
                    break;
            }

            if(addr) { 
                // 保存记录
                result.insert(std::make_pair(next->ifa_name,
                            std::make_pair(addr, prefix_len)));
            }
        }
    } catch (...) {
        SYLAR_LOG_ERROR(g_logger) << "Address::GetInterfaceAddresses exception";
        freeifaddrs(results);
        return false;
    }
    freeifaddrs(results);
    return !result.empty();
}

bool Address::GetInterfaceAddresses(std::vector<std::pair<Address::ptr, uint32_t> >&result
                    ,const std::string& iface, int family) {
    if(iface.empty() || iface == "*") {
        if(family == AF_INET || family == AF_UNSPEC) {
            result.push_back(std::make_pair(Address::ptr(new IPv4Address()), 0u));  // 返回0.0.0.0
        }
        if(family == AF_INET6 || family == AF_UNSPEC) {
            result.push_back(std::make_pair(Address::ptr(new IPv6Address()), 0u));
        }
        return true;
    }

    std::multimap<std::string,std::pair<Address::ptr, uint32_t> > results;

    if(!GetInterfaceAddresses(results, family)) {
        return false;
    }

    auto its = results.equal_range(iface);      // 根据key值(网卡名字)找对应网卡
    for(; its.first != its.second; ++its.first) {
        result.push_back(its.first->second);    // 获得结构
    }
    return !result.empty();
}

Address::ptr Address::Create(const sockaddr* addr, socklen_t addrlen) {
    if(addr == nullptr) {
        return nullptr;
    }

    Address::ptr result;
    switch(addr->sa_family) {
        case AF_INET:
            result.reset(new IPv4Address(*(const sockaddr_in*)addr));
            break;
        case AF_INET6:
            result.reset(new IPv6Address(*(const sockaddr_in6*)addr));
            break;
        default:
            result.reset(new UnknownAddress(*addr));
            break;
    }
    return result;
}


int Address::getFamily() const {
    return getAddr()->sa_family;
}

std::string Address::toString() const {
    std::stringstream ss;
    insert(ss);  // 每个地址提供insert函数，将信息插入string流中
    return ss.str();
}

bool Address::operator<(const Address& rhs) const {
    socklen_t minlen = std::min(getAddrLen() ,rhs.getAddrLen());
    int result = memcmp(getAddr(), rhs.getAddr(), minlen);      // 比较内存块

    if (result < 0) {
        return true;
    } else if (result > 0) {
        return false;
    } else if (getAddrLen() < rhs.getAddrLen()) {
        return true;
    }
    return false;
}
bool Address::operator==(const Address& rhs) const {
    return getAddrLen() == rhs.getAddrLen() &&
           memcmp(getAddr(), rhs.getAddr(), getAddrLen()) == 0;
}
bool Address::operator!=(const Address& rhs) const {
    return !(*this == rhs);
}

IPAddress::ptr IPAddress::Create(const char* address, uint16_t port) {
    addrinfo hints, *results;
    memset(&hints, 0, sizeof(addrinfo));

    // hints.ai_flags = AI_NUMERICHOST;    // address只能是数字化的地址字符串，不能是域名
    hints.ai_family = AF_UNSPEC;        // 允许IPv4 or IPv6

    /// Pv6中引入了getaddrinfo()的新API，它是协议无关的，既可用于IPv4也可用于IPv6。
    /// getaddrinfo函数能够处理名字到地址以及服务到端口这两种转换
    /// !!!!
    int error = getaddrinfo(address, NULL, &hints, &results);
    if(error) {
        SYLAR_LOG_DEBUG(g_logger) << "IPAddress::Create(" << address
            << ", " << port << ") error=" << error
            << " errno=" << errno << " errstr=" << strerror(errno);
        return nullptr;
    }

    try {
        IPAddress::ptr result = std::dynamic_pointer_cast<IPAddress>(
                Address::Create(results->ai_addr, (socklen_t)results->ai_addrlen) 
                // ai_addr是sockaddr类型, 所以需要调用最底层基类的函数来创建通用的对象
        );
        // 如果ai_addr是Unknow类型，dynamic_pointer_cast会转失败，返回Null
        if(result) {
            result->setPort(port);
        }
        freeaddrinfo(results);
        return result;
    } catch (...) {
        freeaddrinfo(results);
        return nullptr;
    }
}

IPv4Address::ptr IPv4Address::Create(const char* address, uint16_t port) {
    IPv4Address::ptr rt(new IPv4Address);
    rt->m_addr.sin_port = byteswapOnLittleEndian(port);
    int result = inet_pton(AF_INET, address, &rt->m_addr.sin_addr);
    if(result <= 0) {
        SYLAR_LOG_DEBUG(g_logger) << "IPv4Address::Create(" << address << ", "
                << port << ") rt=" << result << " errno=" << errno
                << " errstr=" << strerror(errno);
        return nullptr;
    }
    return rt;
}

IPv4Address::IPv4Address(const sockaddr_in& address) {
    m_addr = address;
}
IPv4Address::IPv4Address(uint32_t address, uint16_t port) {
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sin_family = AF_INET;
    m_addr.sin_port = byteswapOnLittleEndian(port);     // 只有小端序的主机需要转换
    m_addr.sin_addr.s_addr = byteswapOnLittleEndian(address);  //??IPv4地址通常是给的是字符??
}

const sockaddr* IPv4Address::getAddr() const {
    return (sockaddr*)&m_addr;

}
sockaddr* IPv4Address::getAddr() {
    return (sockaddr*)&m_addr;
}
socklen_t IPv4Address::getAddrLen() const {
    return sizeof(m_addr);

}
std::ostream& IPv4Address::insert(std::ostream&os ) const {
    uint32_t addr = byteswapOnLittleEndian(m_addr.sin_addr.s_addr);

    /// 这里有inet_ntoa()进行转换为什们不使用???
    os << ((addr >> 24) & 0xff) << "."
       << ((addr >> 16) & 0xff) << "."
       << ((addr >> 8) & 0xff) << "."
       << (addr & 0xff);
    os << ":" << byteswapOnLittleEndian(m_addr.sin_port);
    return os;

}

IPAddress::ptr IPv4Address::broadcastAddress(uint32_t prefix_len) {
    if (prefix_len > 32)
        return nullptr;
    
    sockaddr_in baddr(m_addr);
    // prefix_len ??? 是网络前缀吗??
    baddr.sin_addr.s_addr |= byteswapOnLittleEndian(CreateMask<uint32_t>(prefix_len));

    return IPv4Address::ptr(new IPv4Address(baddr));
}
IPAddress::ptr IPv4Address::networdAddress(uint32_t prefix_len) {
    /// 这里应该不是网段，理解成主机号比较对????
    if(prefix_len > 32) {
        return nullptr;
    }

    sockaddr_in baddr(m_addr);
    baddr.sin_addr.s_addr &= byteswapOnLittleEndian(    // 注意这里还调用了byte....
            CreateMask<uint32_t>(prefix_len));
    /// ??? 这里需要取反吧
    //baddr.sin_addr.s_addr = ~baddr.sin_addr.s_addr;
    return IPv4Address::ptr(new IPv4Address(baddr));

}
IPAddress::ptr IPv4Address::subnetMask(uint32_t prefix_len) {
    /// 子网
    sockaddr_in subnet;
    memset(&subnet, 0, sizeof(subnet));

    subnet.sin_family = AF_INET;
    subnet.sin_addr.s_addr = ~byteswapOnLittleEndian(CreateMask<uint32_t>(prefix_len));
    return IPv4Address::ptr(new IPv4Address(subnet));
}

uint16_t IPv4Address::getPort() const {
    return byteswapOnLittleEndian(m_addr.sin_port);

}
void IPv4Address::setPort(uint16_t v) {
    m_addr.sin_port = byteswapOnLittleEndian(v);
}

IPv6Address::ptr IPv6Address::Create(const char* address, uint16_t port) {
    IPv6Address::ptr rt(new IPv6Address);
    rt->m_addr.sin6_port = byteswapOnLittleEndian(port);
    int result = inet_pton(AF_INET6, address, &rt->m_addr.sin6_addr);
    if(result <= 0) {
        SYLAR_LOG_DEBUG(g_logger) << "IPv6Address::Create(" << address << ", "
                << port << ") rt=" << result << " errno=" << errno
                << " errstr=" << strerror(errno);
        return nullptr;
    }
    return rt;
}

IPv6Address::IPv6Address() {
    memset(&m_addr, 0 , sizeof(m_addr));
    m_addr.sin6_family = AF_INET6;
}

IPv6Address::IPv6Address(const sockaddr_in6& address) {
    m_addr = address;
}

IPv6Address::IPv6Address(const uint8_t address[16], uint16_t port) {
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sin6_family = AF_INET6;
    m_addr.sin6_port = byteswapOnLittleEndian(port);     // 只有小端序的主机需要转换
    memcpy(&m_addr.sin6_addr.s6_addr, address, 16);

}

const sockaddr* IPv6Address::getAddr() const {
    return (sockaddr*)&m_addr;
}
sockaddr* IPv6Address::getAddr() {
    return (sockaddr*)&m_addr;
}
socklen_t IPv6Address::getAddrLen() const {
    return sizeof(m_addr);
}

std::ostream& IPv6Address::insert(std::ostream&os ) const {
    /// ipv6地址转换可以使用inet_ntop转换为字符串
    os << "[";
    // 一个地址块两字节，所以将uint8转化为uint16
    uint16_t* addr = (uint16_t*)m_addr.sin6_addr.s6_addr;
    bool used_zeros = false;
    for(size_t i = 0; i < 8; ++i) {
        if(addr[i] == 0 && !used_zeros) {
            continue;
        }
        if(i && addr[i - 1] == 0 && !used_zeros) {
            os << ":";
            used_zeros = true;
        }
        if(i) {
            os << ":";
        }
        os << std::hex << (int)byteswapOnLittleEndian(addr[i]) << std::dec;
    }

    if(!used_zeros && addr[7] == 0) {
        os << "::";
    }

    os << "]:" << byteswapOnLittleEndian(m_addr.sin6_port);
    return os;
}

IPAddress::ptr IPv6Address::broadcastAddress(uint32_t prefix_len) {
    //??? IPv6要稍微研究一下
    sockaddr_in6 baddr(m_addr);
    baddr.sin6_addr.s6_addr[prefix_len / 8] |=
        CreateMask<uint8_t>(prefix_len % 8);

    for(int i = prefix_len / 8 + 1; i < 16; ++i) {
        baddr.sin6_addr.s6_addr[i] = 0xff;
    }
    return IPv6Address::ptr(new IPv6Address(baddr));
}
IPAddress::ptr IPv6Address::networdAddress(uint32_t prefix_len) {
    ///??? 具体逻辑先跳过
    sockaddr_in6 baddr(m_addr);
    baddr.sin6_addr.s6_addr[prefix_len / 8] &=
        CreateMask<uint8_t>(prefix_len % 8);
    for(int i = prefix_len / 8 + 1; i < 16; ++i) {
        baddr.sin6_addr.s6_addr[i] = 0x00;
    }
    return IPv6Address::ptr(new IPv6Address(baddr));
}
IPAddress::ptr IPv6Address::subnetMask(uint32_t prefix_len) {
    /// 具体逻辑先跳过
    sockaddr_in6 subnet;
    memset(&subnet, 0, sizeof(subnet));
    subnet.sin6_family = AF_INET6;
    subnet.sin6_addr.s6_addr[prefix_len /8] =
        ~CreateMask<uint8_t>(prefix_len % 8);

    for(uint32_t i = 0; i < prefix_len / 8; ++i) {
        subnet.sin6_addr.s6_addr[i] = 0xff;
    }
    return IPv6Address::ptr(new IPv6Address(subnet));
}

uint16_t IPv6Address::getPort() const {
    return byteswapOnLittleEndian(m_addr.sin6_port);    // 我们希望转换为大端，所以只需要在小端机器上进行转换即可
}
void IPv6Address::setPort(uint16_t v) {
    m_addr.sin6_port = byteswapOnLittleEndian(v);
}

static const size_t MAX_PATH_LEN = sizeof(((sockaddr_un*)0)->sun_path) - 1; // -1，去掉\0

UnixAddress::UnixAddress() {
    memset(&m_addr, 0 , sizeof(m_addr));
    m_addr.sun_family = AF_UNIX;
    m_length = offsetof(sockaddr_un, sun_path) + MAX_PATH_LEN;  // 整个结构体的最大长度
    // offsetof: 计算偏移量
    // m_length

}
UnixAddress::UnixAddress(const std::string& path){       // Unix套接字实际是一个文件
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sun_family = AF_UNIX;
    m_length = path.size() + 1;     //  注意加1

    if (!path.empty() && path[0] == '\0') { // '\0'开头的是什么地址会这样??
        --m_length;
    }

    if (m_length > sizeof(m_addr.sun_path)) {
        throw std::logic_error("path too long");
    }
    
    memcpy(m_addr.sun_path, path.c_str(), m_length);
    m_length += offsetof(sockaddr_un, sun_path);    // m_length记录的是sockaddr_un的长度实际长度???
}
const sockaddr* UnixAddress::getAddr() const {
    return (sockaddr*)&m_addr;
}
sockaddr* UnixAddress::getAddr() {
    return (sockaddr*)&m_addr;
}
socklen_t UnixAddress::getAddrLen() const {
    return m_length;
}
void UnixAddress::setAddrLen(uint32_t v) {
    m_length = v;
}
std::ostream& UnixAddress::insert(std::ostream&os ) const {
     if(m_length > offsetof(sockaddr_un, sun_path)
            && m_addr.sun_path[0] == '\0') {

        return os << "\\0"  // 特殊处理，转义一下
                  << std::string(m_addr.sun_path + 1, m_length - offsetof(sockaddr_un, sun_path) - 1);
    }
    // 不是\0开头，则直接输出地址即可
    return os << m_addr.sun_path;       
}

UnknownAddress::UnknownAddress(const sockaddr& addr){
    m_addr = addr;
}

UnknownAddress::UnknownAddress(int family) {
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sa_family = family;
}

const sockaddr* UnknownAddress::getAddr() const {
    return &m_addr;
}
sockaddr* UnknownAddress::getAddr() {
    return &m_addr;
}
socklen_t UnknownAddress::getAddrLen() const {
    return sizeof(m_addr);
}
std::ostream& UnknownAddress::insert(std::ostream& os) const{
    os << "[UnknownAddress family=" << m_addr.sa_family << "]";
    return os;
}



std::ostream& operator<<(std::ostream& os, const Address& addr) {
    return addr.insert(os);
}
} // syalr