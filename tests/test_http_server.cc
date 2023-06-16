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
    // 添加针对url的Servlet
    auto sd = server->getServletDispatch();
    sd->addServlet("/sylar/xx" , [] (sylar::http::HttpRequest::ptr req
                ,sylar::http::HttpResponse::ptr rsp
                ,sylar::http::HttpSession::ptr session){
        rsp->setBody(req->toString());
        return 0;
    });

    sd->addGlobServlet("/sylar/*", [](sylar::http::HttpRequest::ptr req
                ,sylar::http::HttpResponse::ptr rsp
                ,sylar::http::HttpSession::ptr session) {
            rsp->setBody("Glob:\r\n" + req->toString());
            return 0;
    });
    server->start();        // 启动服务器
    
}


int main () {
    sylar::IOManager iom(2);
    iom.schedule(run);

    return 0;
}