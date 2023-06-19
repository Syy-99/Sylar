#include "../sylar/http/http_server.h"
#include "../sylar/log.h"

sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void run() {
    g_logger->setLevel(sylar::LogLevel::INFO);

    sylar::Address::ptr addr = sylar::Address::LookupAnyIPAddress("0.0.0.0:8020");
    if(!addr) {
        SYLAR_LOG_ERROR(g_logger) << "get address error";
        return;
    }

    sylar::http::HttpServer::ptr http_server(new sylar::http::HttpServer());
    //sylar::http::HttpServer::ptr http_server(new sylar::http::HttpServer(true));      // 使用长连接??? 这是服务器怎么影响客户的长连接呀???
    while(!http_server->bind(addr)) {
        SYLAR_LOG_ERROR(g_logger) << "bind " << *addr << " fail";
        sleep(1);
    }
    http_server->start();       // 启动http服务器
}

int main(int argc, char** argv) {
    sylar::IOManager iom(1);
    iom.schedule(run);
    return 0;
}
