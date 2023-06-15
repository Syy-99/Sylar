
#include "tcp_server.h"
#include "config.h"
#include "log.h"

namespace sylar {

static sylar::ConfigVar<uint64_t>::ptr g_tcp_server_read_timeout =
            sylar::Config::Lookup("tcp_server.read_timeout", (uint64_t)(60 * 1000 * 2),
            "tcp server read timeout");

static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

TcpServer::TcpServer(sylar::IOManager* worker, sylar::IOManager* accept_worker)
    :m_worker(worker)
    ,m_acceptWorker(accept_worker)
    ,m_recvTimeout(g_tcp_server_read_timeout->getValue())
    ,m_name("sylar/1.0.0")
    ,m_isStop(true) {
}

TcpServer::~TcpServer() {
    // 回收socket
    for(auto& i : m_socks) {
        i->close();    // 好像没必要，会自动执行
    }
    m_socks.clear();        // 必须要清空，保证引用计数为0， 调用析构

    // ??? 那accept套接字呢，不需要管理吗???
}

bool TcpServer::bind(sylar::Address::ptr addr) {
    std::vector<Address::ptr> addrs;
    addrs.push_back(addr);

    std::vector<Address::ptr> fails;
    return bind(addrs, fails);
}
bool TcpServer::bind(const std::vector<Address::ptr>& addrs,
                     std::vector<Address::ptr>& fails) {
    for(auto& addr : addrs) {
        Socket::ptr sock = Socket::CreateTCP(addr);  // 创建一个TCPsocket
        if (!sock->bind(addr)) {    // 绑定监听地址
            SYLAR_LOG_ERROR(g_logger) << "bind fail errno="
                << errno << " errstr=" << strerror(errno)
                << " addr=[" << addr->toString() << "]";
            fails.push_back(addr);
            continue;
        }

        // 地址绑定成功，开始监听
        if (!sock->listen()) {
            SYLAR_LOG_ERROR(g_logger) << "listen fail errno="
                << errno << " errstr=" << strerror(errno)
                << " addr=[" << addr->toString() << "]";
            fails.push_back(addr);
            continue;
        }
        m_socks.push_back(sock);        /// 保存监听套接字
    }

    if(!fails.empty()) {
        m_socks.clear();    // 所有都clear，即指定的地址必须都成功，
        return false;
    }

    for(auto& i : m_socks) {
        SYLAR_LOG_INFO(g_logger) << " server bind success: " << *i;
    }
    return true;
}

void TcpServer::startAccept(Socket::ptr sock) {
    while(!m_isStop) {
        Socket::ptr client = sock->accept();    // accept最终调用hook过的accept, 默认没有超时时间，即-1
        if(client) {    // 成功连接
            client->setRecvTimeout(m_recvTimeout);      // 设置连接套接字的的读超时时间，即在一段时间没有读事件就关闭
            // 并且将这个连接加入调度，以管理其中的读写事件(通过TcpServer::handleClient函数处理)
            m_worker->schedule(std::bind(&TcpServer::handleClient,
                        shared_from_this(), client));
        } else {
            // 没有连接 or 连接失败，则打印日志信息
            SYLAR_LOG_ERROR(g_logger) << "accept errno=" << errno
                << " errstr=" << strerror(errno);
        }
    }
}

/// 启动服务器
bool TcpServer::start() {
    if (!m_isStop) {
        return true;       // 已经启动，则直接返回
    }

    m_isStop = false;   // 开始启动服务器
    // 对每个监听套接字, 执行accept
    for(auto& sock : m_socks) {
        m_acceptWorker->schedule(std::bind(&TcpServer::startAccept,
                    shared_from_this(), sock));
    }
    return true;

}

void TcpServer::stop() {
    m_isStop = true;

    auto self = shared_from_this();     // 这里拿出引用，防止在stop时被释放or被析构 ??? 没太懂视频说的意思??
    m_acceptWorker->schedule([this, self]() {
        for(auto& sock : m_socks) {
            sock->cancelAll();      // 取消监听socket上的所有事件(事件在哪被初始化了????)
            sock->close();          // 关闭socket
        }
        m_socks.clear();        // 情况
    });

    // stop怎么使用???
}

/// 当用户连接上来，就会执行该函数
void TcpServer::handleClient(Socket::ptr client) {
    // 具体怎么处理用户连接由使用该框架的程序员使用
    SYLAR_LOG_INFO(g_logger) << "handleClient: " << *client;
} 
}