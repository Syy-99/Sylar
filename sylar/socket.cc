#include "socket.h"
#include "fd_manager.h"
#include "logger.h"
#include "macro.h"
sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");
namespace sylar {

Socket::Socket(int family, int type, int protocol) 
    :m_sock(-1)
    ,m_family(family)
    ,m_type(type)
    ,m_protocol(protocol)
    ,m_isConnected(false)
{
    // 注意这个时候还没有socke套接字分配
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
void Socket::setSendTimeout(int64_t, v) {
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
void Socket::setRecvTimeout(int64_t) {
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
   return true

}

bool Socket::setOption(int level, int option, const void* result, socklen_t len) {
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
}

/// 
Socket::ptr accept() {
    Socket::ptr sock(new Socket(m_family, m_type, m_protocol));

    int newsock = ::accept(m_sock, nullptr, nullptr);   // 使用全局的accept函数
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
bool Socket::connect(const Address::ptr addr, uint64_t timeout_ms = -1);

bool Socket::listen(int backlog = SOMAXCONN);   // SOMAXCONN宏

bool Socket::close();

int Socket::send(const void* buffer, size_t length, int flags = 0);
int Socket::send(const iovec* buffers, size_t length, int flags = 0);
int Socket::sendTo(const void* buffer, size_t length, const Address::ptr to, int flags = 0);
int Socket::sendTo(const iovec* buffers, size_t length, const Address::ptr to, int flags = 0);

int Socket::recv(void* buffer, size_t length, int flags = 0);
int Socket::recv(iovec* buffers, size_t length, int flags = 0);
int Socket::recvFrom(void* buffer, size_t length, Address::ptr from, int flags = 0);
int Socket::recvFrom(iovec* buffers, size_t length, Address::ptr from, int flags = 0);

Address::ptr Socket::getRemoteAddress();
Address::ptr Socket::getLocalAddress();

bool Socket::isValid() const;
int Socket::getError();

/// 输出信息的流中
std::ostream& Socket::dump(std::ostream& os) const;

/// 返回socket句柄
int Socket::getSocket() const { return m_sock;}

// socket上的事件管理
bool Socket::cancelRead();
bool Socket::cancelWrite();
bool Socket::cancelAccept();
bool Socket::cancelAll();



void Socket::initSock() {
    // 给一个新的socket设置属性
    int val = 1;
    setOption(SOL_SOCKET, SO_REUSEADDR, val);      // SO_REUSEADDR 设置端口可以立刻重用
    if(m_type == SOCK_STREAM) {
        setOption(IPPROTO_TCP, TCP_NODELAY, val);   //   设置TCP_NODELAY 属性，禁用 Nagle 算法
    }    
}

void Socket::newSock() {
    /// 生成一个socket
    m_sock = socket(m_family, m_type, m_protocol);
    if(SYLAR_LIKELY(m_sock != -1)) {        // m_sock != -1大概率是真的
        initSock();
    } else {
        SYLAR_LOG_ERROR(g_logger) << "socket(" << m_family
            << ", " << m_type << ", " << m_protocol << ") errno="
            << errno << " errstr=" << strerror(errno);
    }
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


}