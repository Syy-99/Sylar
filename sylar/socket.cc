#include "socket.h"
#include "iomanager.h"
#include "fd_manager.h"
#include "log.h"
#include "macro.h"
#include "hook.h"

namespace sylar {

static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

Socket::ptr Socket::CreateTCP(sylar::Address::ptr address) {
    Socket::ptr sock(new Socket(address->getFamily(), TCP, 0));
    return sock;
}

Socket::ptr Socket::CreateUDP(sylar::Address::ptr address) {
    Socket::ptr sock(new Socket(address->getFamily(), UDP, 0));
    /// 如果是UDP套接字，则默认已连接
    sock->newSock();    // ??? 为什们要提前创建socket套接字
    sock->m_isConnected = true;
    return sock;
}

Socket::ptr Socket::CreateTCPSocket() {
    Socket::ptr sock(new Socket(IPv4, TCP, 0));
    return sock;
}

Socket::ptr Socket::CreateUDPSocket() {
    Socket::ptr sock(new Socket(IPv4, UDP, 0));
    sock->newSock();
    sock->m_isConnected = true;
    return sock;
}

Socket::ptr Socket::CreateTCPSocket6() {
    Socket::ptr sock(new Socket(IPv6, TCP, 0));
    return sock;
}

Socket::ptr Socket::CreateUDPSocket6() {
    Socket::ptr sock(new Socket(IPv6, UDP, 0));
    sock->newSock();
    sock->m_isConnected = true;
    return sock;
}

Socket::ptr Socket::CreateUnixTCPSocket() {
    Socket::ptr sock(new Socket(UNIX, TCP, 0));
    return sock;
}

/// ???UNIX套接字创建了有什么用? 为什么它也需要分类呢??
Socket::ptr Socket::CreateUnixUDPSocket() {
    Socket::ptr sock(new Socket(UNIX, UDP, 0));
    return sock;
}

Socket::Socket(int family, int type, int protocol) 
    :m_sock(-1)
    ,m_family(family)
    ,m_type(type)
    ,m_protocol(protocol)
    ,m_isConnected(false)
{
    // 注意这个时候还没有socke套接字分配,只有在操作时才会创建
}
Socket::~Socket() {
    close();        // 调用成员函数close()
}

int64_t Socket::getSendTimeout() {
    FdCtx::ptr ctx = FdMgr::GetInstance()->get(m_sock);     // 中fd管理类中获得对应的socket套接字
    if (ctx) {
        return ctx->getTimeout(SO_SNDTIMEO);
    }
    return -1;
}
void Socket::setSendTimeout(int64_t v) {
    // 毫秒级别
    struct timeval tv{int(v / 1000), int(v % 1000 * 1000)};
    setOption(SOL_SOCKET, SO_SNDTIMEO, tv);     // 调用成员函数setOption, 给套接字设置send超时时间
}

int64_t Socket::getRecvTimeout() {
    FdCtx::ptr ctx = FdMgr::GetInstance()->get(m_sock);     // 中fd管理类中获得对应的socket套接字
    if (ctx) {
        return ctx->getTimeout(SO_RCVTIMEO);
    }
    return -1;
}
void Socket::setRecvTimeout(int64_t v) {
    // 毫秒级别
    struct timeval tv{int(v / 1000), int(v % 1000 * 1000)};
    setOption(SOL_SOCKET, SO_RCVTIMEO, tv);     // 调用成员函数setOption, 给套
}

bool Socket::getOption(int level, int option, void* result, size_t* len) {
    /// 基于Hook的getSocketOption
   int rt = getsockopt(m_sock, level, option, result, (socklen_t*)len);
   if (rt) {
        SYLAR_LOG_DEBUG(g_logger) << "getOption sock=" << m_sock
            << " level=" << level << " option=" << option
            << " errno=" << errno << " errstr=" << strerror(errno);
        return false;
   }
   return true;

}

bool Socket::setOption(int level, int option, const void* result, size_t len) {
    if(setsockopt(m_sock, level, option, result, (socklen_t)len)) {
        SYLAR_LOG_DEBUG(g_logger) << "setOption sock=" << m_sock
            << " level=" << level << " option=" << option
            << " errno=" << errno << " errstr=" << strerror(errno);
        return false;
    }
    return true;
}

bool Socket::bind(const Address::ptr addr) {
    // 基于Hook的bind
    if (!isValid()) {
        newSock();  // 注意，在这时才初始化一个socket套接字保存在对象中
        if(SYLAR_UNLIKELY(!isValid())) {     // !isValid更有可能是0，即更有可能不执行该条件
            return false;
        }
    }

    // ?? 只需要判断family吗????
    if (SYLAR_UNLIKELY(addr->getFamily() != m_family)) {  // 绑定的地址不符合该socket的类型
        SYLAR_LOG_ERROR(g_logger) << "bind sock.family("
            << m_family << ") addr.family(" << addr->getFamily()
            << ") not equal, addr=" << addr->toString();
        return false;
    }

    if(::bind(m_sock, addr->getAddr(), addr->getAddrLen())) {
        SYLAR_LOG_ERROR(g_logger) << "bind error errrno=" << errno
            << " errstr=" << strerror(errno);
        return false;
    }
    getLocalAddress();      // 初始化本地地址
    return true;

}

Socket::ptr Socket::accept() {
    Socket::ptr sock(new Socket(m_family, m_type, m_protocol));

    int newsock = ::accept(m_sock, nullptr, nullptr); 
    if(newsock == -1) {
        SYLAR_LOG_ERROR(g_logger) << "accept(" << m_sock << ") errno="
            << errno << " errstr=" << strerror(errno);
        return nullptr;
    }

    // 根据accept返回的sock的文件描述符，初始化我们的Socket对象
    if(sock->init(newsock)) {
        return sock;
    }
    return nullptr;
}
bool Socket::connect(const Address::ptr addr, uint64_t timeout_ms) {
    if (!isValid()) {
        newSock();
        if(SYLAR_UNLIKELY(!isValid())) {
            return false;
        }
    }
    // SYLAR_LOG_INFO(g_logger) << "???1" << m_sock;
    if(SYLAR_UNLIKELY(addr->getFamily() != m_family)) {
        SYLAR_LOG_ERROR(g_logger) << "connect sock.family("
            << m_family << ") addr.family(" << addr->getFamily()
            << ") not equal, addr=" << addr->toString();
        return false;
    }
    // SYLAR_LOG_INFO(g_logger) << "???2";

    if (timeout_ms == (uint64_t)-1) {
        // SYLAR_LOG_INFO(g_logger) << "??? begin";
         if(::connect(m_sock, addr->getAddr(), addr->getAddrLen())) {
            SYLAR_LOG_ERROR(g_logger) << "sock=" << m_sock << " connect(" << addr->toString()
                << ") error errno=" << errno << " errstr=" << strerror(errno);
            close();
            return false;
        }
        // SYLAR_LOG_INFO(g_logger) << "??? end";
    } else {
        
        if(::connect_with_timeout(m_sock, addr->getAddr(), addr->getAddrLen(), timeout_ms)) {
            SYLAR_LOG_ERROR(g_logger) << "sock=" << m_sock << " connect(" << addr->toString()
                << ") timeout=" << timeout_ms << " error errno="
                << errno << " errstr=" << strerror(errno);
            close();
            return false;
        }
    }
    // SYLAR_LOG_INFO(g_logger) << "???3";
    m_isConnected = true;
    getLocalAddress();      
    getRemoteAddress();     // 连接地址就是远程地址
    return true;

}

bool Socket::listen(int backlog) {   // SOMAXCONN宏
    if(!isValid()) {
        SYLAR_LOG_ERROR(g_logger) << "listen error sock=-1";
        return false;
    }

    if(::listen(m_sock, backlog)) {
        SYLAR_LOG_ERROR(g_logger) << "listen error errno=" << errno
            << " errstr=" << strerror(errno);
        return false;
    }
    return true;

}

bool Socket::close() {
    // m_socket = -1 为什么返回true呢?
    if(!m_isConnected && m_sock == -1) {
        return true;
    }

    m_isConnected = false;
    if(m_sock != -1) {
        ::close(m_sock);
        m_sock = -1;
    }

    return false;    // ?? 返回false???
}

int Socket::send(const void* buffer, size_t length, int flags) {
    if(isConnected()) {
        return ::send(m_sock, buffer, length, flags);
    }
    return -1;    
}

// 多个buffer一起发送
int Socket::send(const iovec* buffers, size_t length, int flags) {
    if(isConnected()) {
        msghdr msg;
        memset(&msg, 0, sizeof(msg));
        msg.msg_iov = (iovec*)buffers;
        msg.msg_iovlen = length;

        return ::sendmsg(m_sock, &msg, flags);
    }
    return -1;
}
int Socket::sendTo(const void* buffer, size_t length, const Address::ptr to, int flags) {
    if(isConnected()) {
        return ::sendto(m_sock, buffer, length, flags, to->getAddr(), to->getAddrLen());
    }
    return -1;
}
int Socket::sendTo(const iovec* buffers, size_t length, const Address::ptr to, int flags) {
    if(isConnected()) {
        msghdr msg;
        memset(&msg, 0, sizeof(msg));
        msg.msg_iov = (iovec*)buffers;
        msg.msg_iovlen = length;
        msg.msg_name = to->getAddr();       // UDP需要地址信息 msg.msg_name非const的类型
        msg.msg_namelen = to->getAddrLen();

        return ::sendmsg(m_sock, &msg, flags);
    }
    return -1;
}

int Socket::recv(void* buffer, size_t length, int flags ) {
    if(isConnected()) {
        return ::recv(m_sock, buffer, length, flags);
    }
    return -1;
}
int Socket::recv(iovec* buffers, size_t length, int flags) {
    if(isConnected()) {
        msghdr msg;
        memset(&msg, 0, sizeof(msg));
        msg.msg_iov = (iovec*)buffers;      /// 保存一个iovec数组，用来接受数据
        msg.msg_iovlen = length;
        return ::recvmsg(m_sock, &msg, flags);
    }
    return -1;
}
int Socket::recvFrom(void* buffer, size_t length, Address::ptr from, int flags) {
    if(isConnected()) {
        socklen_t len = from->getAddrLen();
        return ::recvfrom(m_sock, buffer, length, flags, from->getAddr(), &len);
    }
    return -1;
}
int Socket::recvFrom(iovec* buffers, size_t length, Address::ptr from, int flags) {
    if(isConnected()) {
        msghdr msg;
        memset(&msg, 0, sizeof(msg));
        msg.msg_iov = (iovec*)buffers;
        msg.msg_iovlen = length;
        msg.msg_name = from->getAddr();
        msg.msg_namelen = from->getAddrLen();
        return ::recvmsg(m_sock, &msg, flags);
    }
    return -1;
}

Address::ptr Socket::getRemoteAddress() {
    ///?? 是否需要判断是否连接???
    if(m_remoteAddress) {       // ??? 在哪里会提前设置了这个呢???
        return m_remoteAddress;
    }

    Address::ptr result;
    switch(m_family) {
        case AF_INET:
            result.reset(new IPv4Address());
            break;
        case AF_INET6:
            result.reset(new IPv6Address());
            break;
        case AF_UNIX:
            result.reset(new UnixAddress());
            break;
        default:
            result.reset(new UnknownAddress(m_family));
            break;
    } 
    
    socklen_t addrlen = result->getAddrLen();  
    if(getpeername(m_sock, result->getAddr(), &addrlen)) {  // getpeername: 获得m_sockt的远端的地址
        SYLAR_LOG_ERROR(g_logger) << "getpeername error sock=" << m_sock
           << " errno=" << errno << " errstr=" << strerror(errno);
        return Address::ptr(new UnknownAddress(m_family));  // 有问题，则返回Unknow地址
    }

    if(m_family == AF_UNIX) {
        // UNIX地址对象有一个len属性需要调用它的成员方法设置，所以需要进行类型转换
        UnixAddress::ptr addr = std::dynamic_pointer_cast<UnixAddress>(result);
        addr->setAddrLen(addrlen);
    }
    m_remoteAddress = result;
    return m_remoteAddress;


}
Address::ptr Socket::getLocalAddress() {
    if(m_localAddress) {        // 之前不会设置吗? 只有在第二次getxxx时才会执行这个if
        return m_localAddress;
    }

    Address::ptr result;
    switch(m_family) {
        case AF_INET:
            result.reset(new IPv4Address());
            break;
        case AF_INET6:
            result.reset(new IPv6Address());
            break;
        case AF_UNIX:
            result.reset(new UnixAddress());
            break;
        default:
            result.reset(new UnknownAddress(m_family));
            break;
    }
    socklen_t addrlen = result->getAddrLen();
    if(getsockname(m_sock, result->getAddr(), &addrlen)) {  // getsockname: 获取本段socket地址
        SYLAR_LOG_ERROR(g_logger) << "getsockname error sock=" << m_sock
            << " errno=" << errno << " errstr=" << strerror(errno);
        return Address::ptr(new UnknownAddress(m_family));
    }
    if(m_family == AF_UNIX) {
        UnixAddress::ptr addr = std::dynamic_pointer_cast<UnixAddress>(result);
        addr->setAddrLen(addrlen);
    }
    m_localAddress = result;
    return m_localAddress;
}

bool Socket::isValid() const {
    return m_sock != -1;
}

int Socket::getError() {
    int error = 0;
    size_t len = sizeof(error);
    /// SO_ERROR 当套接口发生错误时,
    /// 套接口名为so_error的变量被设置为标准的UNIX Exxx值的一个,它称为套接字的待处理错误
    if(!getOption(SOL_SOCKET, SO_ERROR, &error, &len)) {
        error = errno;
    }
    return error;
}

/// 输出信息的流中
std::ostream& Socket::dump(std::ostream& os) const {
    os << "[Socket sock=" << m_sock
       << " is_connected=" << m_isConnected
       << " family=" << m_family
       << " type=" << m_type
       << " protocol=" << m_protocol;

    if(m_localAddress) {
        os << " local_address=" << m_localAddress->toString();
    }

    if(m_remoteAddress) {
        os << " remote_address=" << m_remoteAddress->toString();
    }

    os << "]";
    return os;
}

// socket上的事件管理
bool Socket::cancelRead() {
    // 取消m_sock上的读事件
    /// 所有的事件都会添加到IOManager中，根据m_sock管理
    return IOManager::GetThis()->cancelEvent(m_sock, sylar::IOManager::READ);
}

bool Socket::cancelWrite() {
    return IOManager::GetThis()->cancelEvent(m_sock, sylar::IOManager::WRITE);
}

bool Socket::cancelAccept() {
    return IOManager::GetThis()->cancelEvent(m_sock, sylar::IOManager::READ);
}

bool Socket::cancelAll() {
    return IOManager::GetThis()->cancelAll(m_sock);
}


void Socket::initSock() {
    // 给一个新的socket设置属性
    int val = 1;
    setOption(SOL_SOCKET, SO_REUSEADDR, val);      // SO_REUSEADDR 设置端口可以立刻重用
    if(m_type == SOCK_STREAM) {
        setOption(IPPROTO_TCP, TCP_NODELAY, val);   //   设置TCP_NODELAY 属性，禁用 Nagle 算法
        // 禁止合并缓存，但是为了降低延迟就得必须发
    }    
}

void Socket::newSock() {
    /// 生成一个socket
    m_sock = socket(m_family, m_type, m_protocol);
    // SYLAR_LOG_ERROR(g_logger) << "new " <<m_sock;
    if(SYLAR_LIKELY(m_sock != -1)) {        // m_sock != -1大概率是真的
        initSock();
    } else {
        SYLAR_LOG_ERROR(g_logger) << "socket(" << m_family
            << ", " << m_type << ", " << m_protocol << ") errno="
            << errno << " errstr=" << strerror(errno);
    }
}  

/**
    * @brief 通过fd的方式初始化sock套接字
    */
bool Socket::init(int sock) {
    FdCtx::ptr ctx = FdMgr::GetInstance()->get(sock);   // 加入管理
     if(ctx && ctx->isSocket() && !ctx->isClose()) {
        m_sock = sock;
        m_isConnected = true;
        initSock();
        getLocalAddress();  // 初始化
        getRemoteAddress();
        return true;
    }
    return false;
}

std::ostream& operator<<(std::ostream& os, const Socket& sock) {
    return sock.dump(os);
}

}