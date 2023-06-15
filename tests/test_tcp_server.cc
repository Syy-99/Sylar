#include "../sylar/tcp_server.h"
#include "../sylar/iomanager.h"
#include "../sylar/log.h"

sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void run() {
    auto addr = sylar::Address::LookupAny("0.0.0.0:8033");  // 返回一个IPv4地址对象
    SYLAR_LOG_INFO(g_logger) << *addr;

    // auto addr2 = sylar::UnixAddress::ptr(new sylar::UnixAddress("/tmp/unix_addr"));
    // SYLAR_LOG_INFO(g_logger) << *addr2;

    std::vector<sylar::Address::ptr> addrs;
    addrs.push_back(addr);
    // addrs.push_back(addr2);

    // 创建TCP server
    sylar::TcpServer::ptr tcp_server(new sylar::TcpServer);
    std::vector<sylar::Address::ptr> fails;
    while(!tcp_server->bind(addrs, fails)) {    // 创建了一系列的监听套接字
        sleep(2);
    }
    tcp_server->start();
}

int main() {
    sylar::IOManager iom(2);     // 创建两个线程
    iom.schedule(run);
    return 0;
}