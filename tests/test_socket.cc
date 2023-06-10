#include "../sylar/socket.h"
#include "../sylar/sylar.h"
#include "../sylar/iomanager.h"


static sylar::Logger::ptr g_looger = SYLAR_LOG_ROOT();


void test_socket() {
    // 1. 获得地址对象
    sylar::IPAddress::ptr addr = sylar::Address::LookupAnyIPAddress("www.baidu.com");
    if(addr) {
        SYLAR_LOG_INFO(g_looger) << "get address: " << addr->toString();
    } else {
        SYLAR_LOG_ERROR(g_looger) << "get address fail";
        return;
    }

    // 2. 根据要连接的地址创建对应类型的套接字
    sylar::Socket::ptr sock = sylar::Socket::CreateTCP(addr);
    addr->setPort(80);  // 设置连接的端口
    SYLAR_LOG_INFO(g_looger) << "addr=" << addr->toString();
    if(!sock->connect(addr)) {      // 连接对应的地址
        SYLAR_LOG_ERROR(g_looger) << "connect " << addr->toString() << " fail";
        return;
    } else {
        SYLAR_LOG_INFO(g_looger) << "connect " << addr->toString() << " connected";
    } 


    // 3. 发送信息
    const char buff[] = "GET / HTTP/1.0\r\n\r\n";
    int rt = sock->send(buff, sizeof(buff));
    if(rt <= 0) {
        SYLAR_LOG_INFO(g_looger) << "send fail rt=" << rt;
        return;
    }

    // 4. 接受信息
    std::string buffs;
    buffs.resize(4096);
    rt = sock->recv(&buffs[0], buffs.size());
    if(rt <= 0) {
        SYLAR_LOG_INFO(g_looger) << "recv fail rt=" << rt;
        return;
    }

    buffs.resize(rt);
    SYLAR_LOG_INFO(g_looger) << buffs;       
}
int main() {
    sylar::IOManager iom;
    iom.schedule(test_socket);    //所有线程都默认启动hook
    return 0;
}