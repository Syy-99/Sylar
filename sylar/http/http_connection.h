/**
 * @file http_connection.h
 * @brief HTTP客户端类

 */
#ifndef __SYLAR_HTTP_CONNECTION_H__
#define __SYLAR_HTTP_CONNECTION_H__


#include "../socket_stream.h"
#include "http.h"
#include "../uri.h"

namespace sylar {
namespace http {


/**
 * @brief HTTP响应结果
 */
struct HttpResult {
    typedef std::shared_ptr<HttpResult> ptr;

    // 状态码
    enum class Error {
        /// 正常
        OK = 0,
        /// 非法URL
        INVALID_URL = 1,
        /// 无法解析HOST
        INVALID_HOST = 2,
        /// 连接失败
        CONNECT_FAIL = 3,
        /// 连接被对端关闭
        SEND_CLOSE_BY_PEER = 4,
        /// 发送请求产生Socket错误
        SEND_SOCKET_ERROR = 5,
        /// 超时
        TIMEOUT = 6,
        /// 创建Socket失败
        CREATE_SOCKET_ERROR = 7,
        /// 从连接池中取连接失败
        POOL_GET_CONNECTION = 8,
        /// 无效的连接
        POOL_INVALID_CONNECTION = 9,
    };

    HttpResult(int _result
               ,HttpResponse::ptr _response
               ,const std::string& _error)
        :result(_result)
        ,response(_response)
        ,error(_error) {}

    /// 状态码
    int result;
    /// HTTP响应结构体
    HttpResponse::ptr response;
    /// 错误描述
    std::string error;

    std::string toString() const;

};
class HttpConnection : public SocketStream {
public:
    typedef std::shared_ptr<HttpConnection> ptr;

    HttpConnection(Socket::ptr sock, bool owner = true);

    // 方便使用
    // 发送HTTP请求
    static HttpResult::ptr DoRequest(HttpMethod method
                        , const std::string& url
                        , uint64_t timeout_ms       // 表示超时
                        , const std::map<std::string, std::string>& headers = {}
                        , const std::string& body = "");

    static HttpResult::ptr DoRequest(HttpMethod method
                        , Uri::ptr uri
                        , uint64_t timeout_ms
                        , const std::map<std::string, std::string>& headers = {}
                        , const std::string& body = "");

    static HttpResult::ptr DoRequest(HttpRequest::ptr req
                        , Uri::ptr uri
                        , uint64_t timeout_ms);
    
    static HttpResult::ptr DoGet(const std::string& url
                        , uint64_t timeout_ms
                        , const std::map<std::string, std::string>& headers = {}
                        , const std::string& body = "");
    static HttpResult::ptr DoGet(Uri::ptr uri
                        , uint64_t timeout_ms
                        , const std::map<std::string, std::string>& headers = {}
                        , const std::string& body = "");

    static HttpResult::ptr DoPost(const std::string& url
                            , uint64_t timeout_ms
                            , const std::map<std::string, std::string>& headers = {}
                            , const std::string& body = "");

    static HttpResult::ptr DoPost(Uri::ptr uri
                            , uint64_t timeout_ms
                            , const std::map<std::string, std::string>& headers = {}
                            , const std::string& body = "");
            

    /// 接收HTTP响应
    HttpResponse::ptr recvResponse();

    /// 发送HTTP请求
    int sendRequest(HttpRequest::ptr rsp);

private:

}; 

}
}

#endif