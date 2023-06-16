/**
 * @file http_server.h
 * @brief HTTP服务器封装
 */

#ifndef __SYLAR_HTTP_HTTP_SERVER_H__
#define __SYLAR_HTTP_HTTP_SERVER_H__


#include "../tcp_server.h"
#include "http_session.h"

namespace sylar {
namespace http {

/**
 * @brief HTTP服务器类
 */
class HttpServer : public TcpServer {
public:
    /// 智能指针类型
    typedef std::shared_ptr<HttpServer> ptr;
    HttpServer(bool keep_alive = false,
               sylar::IOManager* worker = sylar::IOManager::GetThis(),
               sylar::IOManager* accept_worker = sylar::IOManager::GetThis());

protected:
    virtual void handleClient(Socket::ptr client) override;
private:
    /// 是否支持长连接
    bool m_isKeepalive;

};
}
}

#endif
