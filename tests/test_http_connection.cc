#include <iostream>
#include "../sylar/http/http_connection.h"
#include "../sylar/log.h"
#include "../sylar/iomanager.h"

static sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void run() {
    // 创建sockt地址
    sylar::Address::ptr addr = sylar::Address::LookupAnyIPAddress("www.sylar.top:80");
    if(!addr) {
        SYLAR_LOG_INFO(g_logger) << "get addr error";
        return;
    }

    // 创建TCP套接字并连接
    sylar::Socket::ptr sock = sylar::Socket::CreateTCP(addr);
    bool rt = sock->connect(addr);
    if(!rt) {
        SYLAR_LOG_INFO(g_logger) << "connect " << *addr << " failed";
        return;
    }

    // 创建客户端连接套接字对象
    sylar::http::HttpConnection::ptr conn(new sylar::http::HttpConnection(sock));
    
    // 构造HTTP请求报文
    sylar::http::HttpRequest::ptr req(new sylar::http::HttpRequest);
    req->setPath("/blog/");
    req->setHeader("host", "www.sylar.top");
    SYLAR_LOG_INFO(g_logger) << "req:" << std::endl
        << *req;

    // 发送请求报文
    conn->sendRequest(req); 

    // 读取相应报文
    auto rsp = conn->recvResponse();
    if(!rsp) {
        SYLAR_LOG_INFO(g_logger) << "recv response error";
        return;
    }
    SYLAR_LOG_INFO(g_logger) << "rsp:" << std::endl
        << *rsp;   
}

int main() {
    sylar::IOManager iom(2);
    iom.schedule(run);
    return 0;
}