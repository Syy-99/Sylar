#include "http_server.h"
#include "../log.h"

namespace sylar {
namespace http {

static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

HttpServer::HttpServer(bool keep_alive,
                sylar::IOManager* worker,
                sylar::IOManager* accept_worker)
        : TcpServer(worker, accept_worker)
        , m_isKeepalive(keep_alive) {

}


void HttpServer::handleClient(Socket::ptr client) {
    HttpSession::ptr session(new HttpSession(client));
    do {
        auto req = session->recvRequest();
        if (!req) {
            SYLAR_LOG_DEBUG(g_logger) << "recv http request fail, errno="
                << errno << " errstr=" << strerror(errno)
                << " cliet:" << *client << " keep_alive=" << m_isKeepalive;
            break;
        }

        HttpResponse::ptr rsp(new HttpResponse(req->getVersion(), req->isClose() || !m_isKeepalive));

        // 先做简单的
        rsp->setBody("hello Syalr");

        SYLAR_LOG_INFO(g_logger) << "request: " << std::endl
            << *req;

        SYLAR_LOG_INFO(g_logger) << "response: " << std::endl
            << *rsp;
        session->sendResponse(rsp);    
    } while(m_isKeepalive);
    session->close();     // 关闭套接字
}

}
}