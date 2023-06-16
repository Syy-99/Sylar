#include "../sylar/http/http_server.h"
#include "../sylar/log.h"


static sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void run () {

    // 创建HTTP服务器
    sylar::http::HttpServer::ptr server(new sylar::http::HttpServer());
    // 创建socket
    sylar::Address::ptr addr = sylar::Address::LookupAnyIPAddress("0.0.0.0:8020");
    while(!server->bind(addr)) {
        sleep(2);
    }
    server->start();        // 启动服务器
    
}


int main () {
    sylar::IOManager iom(2);
    iom.schedule(run);

    return 0;
}