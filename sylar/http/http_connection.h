/**
 * @file http_connection.h
 * @brief HTTP客户端类

 */
#ifndef __SYLAR_HTTP_CONNECTION_H__
#define __SYLAR_HTTP_CONNECTION_H__


#include "../socket_stream.h"
#include "http.h"
#include <list>
#include "../uri.h"
#include "../mutex.h"
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

class HttpConnectionPool;
class HttpConnection : public SocketStream {
friend class HttpConnectionPool;
public:
    typedef std::shared_ptr<HttpConnection> ptr;

    HttpConnection(Socket::ptr sock, bool owner = true);
    ~HttpConnection();
    
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
    // 给下面连接池用的
    uint64_t m_createTime = 0;     // 这个httpconnection的开始连接的时间
    uint64_t m_request = 0;        // 这个连接中的发起请求的数量
}; 

/// 针对host的连接池
/// ??应该只有设置为keep-alive的连接才会放到连接池中吧?? 代码中哪里体现了???
class HttpConnectionPool {
public:
    typedef std::shared_ptr<HttpConnectionPool> ptr;
    typedef Mutex MutexType;


    HttpConnectionPool(const std::string& host
                       ,const std::string& vhost
                       ,uint32_t port
                       ,uint32_t max_size
                       ,uint32_t max_alive_time
                       ,uint32_t max_request);

    /// 拿连接
    HttpConnection::ptr getConnection();


    HttpResult::ptr doRequest(HttpMethod method
                        , const std::string& url
                        , uint64_t timeout_ms       // 表示超时
                        , const std::map<std::string, std::string>& headers = {}
                        , const std::string& body = "");

    HttpResult::ptr doRequest(HttpMethod method
                        , Uri::ptr uri
                        , uint64_t timeout_ms
                        , const std::map<std::string, std::string>& headers = {}
                        , const std::string& body = "");

    HttpResult::ptr doRequest(HttpRequest::ptr req
                        , uint64_t timeout_ms);
    
    HttpResult::ptr doGet(const std::string& url
                        , uint64_t timeout_ms
                        , const std::map<std::string, std::string>& headers = {}
                        , const std::string& body = "");
    HttpResult::ptr doGet(Uri::ptr uri
                        , uint64_t timeout_ms
                        , const std::map<std::string, std::string>& headers = {}
                        , const std::string& body = "");

    HttpResult::ptr doPost(const std::string& url
                            , uint64_t timeout_ms
                            , const std::map<std::string, std::string>& headers = {}
                            , const std::string& body = "");

    HttpResult::ptr doPost(Uri::ptr uri
                            , uint64_t timeout_ms
                            , const std::map<std::string, std::string>& headers = {}
                            , const std::string& body = "");
private:
    // 提供给智能指针用的析构函数
    static void ReleasePtr(HttpConnection* ptr, HttpConnectionPool* pool);
private:
    std::string m_host;      // ???这个咋用
    std::string m_vhost;    // 虚拟主机
    uint32_t m_port;        // 对应的port??(同协议，端口相同，不同host地址)

    uint32_t m_maxSize;         // 连接池大小（如果因为负载额外创建，则超过部分释放时会直接释放）
    // 仿照Ngnix
    uint32_t m_maxAliveTime;    // 连接最大存在时间
    uint32_t m_maxRequest;      // 每个连接最多处理的请求数量

    MutexType m_mutex;
    std::list<HttpConnection*> m_conns; // 保存连接
    std::atomic<int32_t> m_total = {0}; // 记录连接数量


};


}
}

#endif