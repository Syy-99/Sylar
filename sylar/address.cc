#include <address.h>
#include <sstream>

#include "endian.hj"
namespace sylar {


int Address::getFamily() const {
    return getAddr()->sa_family;
}

std::string Address::toString() {
    std::stringstream ss;
    insert(ss);  // 每个地址提供insert函数，将信息插入string流中
    return ss.str();
}

bool Address::operator<(const Address& rhs) const {
    socklen_t minlen = std::min(getAddrLen() ,rhs.getAddrLen);
    int result = memcmp(getAddr(), rhs.getAddr(), minlen);      // 比较内存块

    if (result < 0) {
        return true;
    } else if (result > 0) {
        return false;
    } else if (getAddrLen() < rhs.getAddrLen) {
        return true;
    }
    return false;
}
bool Address::operator==(const Address& rhs) const {
    return getAddrLen() == rhs.getAddrLen &&
           memcmp(getAddr(), rhs.getAddr(), getAddrLen()) == 0;
}
bool Address::operator!=(const Address& rhs) const {
    return !(*this == rhs);
}

IPv4Address::IPv4Address(uint32_t address, uint32_t port) {
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sin_family = AF_INET;
    m_addr.sin_port = byteswapOnLittleEndian(port);     // 只有小端序的主机需要转换
    m_addr.sin_addr.s_addr = byteswapOnLittleEndian(addrress);  //??IPv4地址通常是给的是字符??
}

const sockaddr* IPv4Address::getAddr() const {
    return (sockaddr*)&m_addr;

}
socklen_t Ipv4Address::getAddrLen() const {
    return sizeof(m_addr);

}
std::ostream& Ipv4Address::insert(std::ostream&os ) const {
    uint32_t addr = byteswapOnLittleEndian(m_addr.sin_addr.s_addr);

    /// 这里有inet_ntoa()进行转换为什们不使用???
    os << ((addr >> 24) & 0xff) << "."
       << ((addr >> 16) & 0xff) << "."
       << ((addr >> 8) & 0xff) << "."
       << (addr & 0xff);
    os << ":" << byteswapOnLittleEndian(m_addr.sin6_port);
    return os;

}

IPAddress::ptr Ipv4Address::broadcastAddress(uint32_t prefix_len) {
    if (prefix_len > 32)
        return nullptr;
    
    sockaddr_in baddr(m_addr);
}
IPAddress::ptr Ipv4Address::networdAddress(uint32_t prefix_len) {

}
IPAddress::ptr Ipv4Address::subnetMask(uint32_t prefix_len) {

}

uint32_t Ipv4Address::getPort() const {
    return byteswapOnLittleEndian(m_addr.sin_port);

}
void Ipv4Address::setPort(uint32_t v) {
    m_addr.sin_port = byteswapOnLittleEndian(v);
}

Ipv6Address::IPv6Address() {
    memset(&m_addr, 0 , sizeof(m_addr));
    m_addr.sinfa = AF_INET6;
}
IPv6Address::IPv6Address(const char* address, uint32_t port) {
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sin_family = AF_INET6;
    m_addr.sin_port = byteswapOnLittleEndian(port);     // 只有小端序的主机需要转换
    memcpy(&m_addr.sin6_addr.s6_addr, address, 16)

}

const sockaddr* IPv6ddress::getAddr() const {
    return (sockaddr*)&m_addr;
}
socklen_t Ipv6Address::getAddrLen() const {
    return sizeof(m_addr);
}

std::ostream& Ipv6Address::insert(std::ostream&os ) const {
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

IPAddress::ptr Ipv6Address::broadcastAddress(uint32_t prefix_len) {

}
IPAddress::ptr Ipv6Address::networdAddress(uint32_t prefix_len) {

}
IPAddress::ptr Ipv6Address::subnetMask(uint32_t prefix_len) {

}

uint32_t Ipv6Address::getPort() const {
    return byteswapOnLittleEndian(m_addr.sin6_port);    // 我们希望转换为大端，所以只需要在小端机器上进行转换即可
}
void Ipv6Address::setPort(uint32_t v) {
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
UnixAddress::UnixAddress(cosnt std::string& path){       // Unix套接字实际是一个文件
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sun_family = AF_UNIX;
    m_length = path.size() + 1;     //  注意加1

    if (!path.empty() && path[0] == '\0') { // '\0'开头的是什么地址会这样??
        --m_length;
    }

    if (m_length > sizeof(m_addr.sun_path)) {
        throw std::logic_error("path too long");
    }
    
    memcpy(m_addr.sun_path, path.c_str, m_length);
    m_length += offsetof(sockaddr_un, sun_path);    // m_length记录的是sockaddr_un的长度实际长度???
}
const sockaddr* UnixAddress::getAddr() const {
    return (sockaddr*)&m_addr;
}
socklen_t UnixAddress::getAddrLen() const {
    return m_length;
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

}

UnknownAddress::UnknownAddress(int family) {
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sa_family = family;
}

const sockaddr* UnknownAddress::getAddr() const {
    return &m_addr;
}
socklen_t UnknownAddress::getAddrLen() const {
    return sizeof(m_addr);
}
std::ostream& UnknownAddress::insert(std::ostream& os) const{
    os << "[UnknownAddress family=" << m_addr.sa_family << "]";
    return os;
}
} // syalr
