/**
 * @file http_session.h
 * @brief HTTPSession封装
  服务端连接套接字
 */

#ifndef __SYLAR_HTTP_SESSION_H__
#define __SYLAR_HTTP_SESSION_H__

#include "../socket_stream.h"
#include "http.h"

namespace sylar {
namespace http {

class HttpSession : public SocketStream {
public:
    typedef std::shared_ptr<HttpSession> ptr;

    HttpSession(Socket::ptr sock, bool owner = true);

    /// 接收HTTP请求
    HttpRequest::ptr recvRequest();

    /// 发送HTTP响应
    int sendResponse(HttpResponse::ptr rsp);

private:

};

}
}

#endif