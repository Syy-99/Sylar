/**
 * @file http_connection.h
 * @brief HTTP客户端类

 */
#ifndef __SYLAR_HTTP_CONNECTION_H__
#define __SYLAR_HTTP_CONNECTION_H__


#include "../socket_stream.h"
#include "http.h"


namespace sylar {
namespace http {

class HttpConnection : public SocketStream {
public:
    typedef std::shared_ptr<HttpConnection> ptr;

    HttpConnection(Socket::ptr sock, bool owner = true);

    /// 接收HTTP响应
    HttpResponse::ptr recvResponse();

    /// 发送HTTP请求
    int sendRequest(HttpRequest::ptr rsp);

private:

}; 

}
}

#endif