/**
 * @file http.h
 * @brief HTTP定义结构体封装
 */

#ifndef __SYLAR_HTTP_HTTP_H__
#define __SYLAR_HTTP_HTTP_H__


#include <memory>
#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <sstream>
#include <boost/lexical_cast.hpp>    // ?? 为什么用boost的类型转换而不用c++的类型转换

namespace sylar {
namespace http {        // ??? 为什么要这里做

/// 封装HTTP请求的方法
#define HTTP_METHOD_MAP(XX)         \
  XX(0,  DELETE,      DELETE)       \
  XX(1,  GET,         GET)          \
  XX(2,  HEAD,        HEAD)         \
  XX(3,  POST,        POST)         \
  XX(4,  PUT,         PUT)          \
  /* pathological */                \
  XX(5,  CONNECT,     CONNECT)      \
  XX(6,  OPTIONS,     OPTIONS)      \
  XX(7,  TRACE,       TRACE)        \
  /* WebDAV */                      \
  XX(8,  COPY,        COPY)         \
  XX(9,  LOCK,        LOCK)         \
  XX(10, MKCOL,       MKCOL)        \
  XX(11, MOVE,        MOVE)         \
  XX(12, PROPFIND,    PROPFIND)     \
  XX(13, PROPPATCH,   PROPPATCH)    \
  XX(14, SEARCH,      SEARCH)       \
  XX(15, UNLOCK,      UNLOCK)       \
  XX(16, BIND,        BIND)         \
  XX(17, REBIND,      REBIND)       \
  XX(18, UNBIND,      UNBIND)       \
  XX(19, ACL,         ACL)          \
  /* subversion */                  \
  XX(20, REPORT,      REPORT)       \
  XX(21, MKACTIVITY,  MKACTIVITY)   \
  XX(22, CHECKOUT,    CHECKOUT)     \
  XX(23, MERGE,       MERGE)        \
  /* upnp */                        \
  XX(24, MSEARCH,     M-SEARCH)     \
  XX(25, NOTIFY,      NOTIFY)       \
  XX(26, SUBSCRIBE,   SUBSCRIBE)    \
  XX(27, UNSUBSCRIBE, UNSUBSCRIBE)  \
  /* RFC-5789 */                    \
  XX(28, PATCH,       PATCH)        \
  XX(29, PURGE,       PURGE)        \
  /* CalDAV */                      \
  XX(30, MKCALENDAR,  MKCALENDAR)   \
  /* RFC-2068, section 19.6.1.2 */  \
  XX(31, LINK,        LINK)         \
  XX(32, UNLINK,      UNLINK)       \
  /* icecast */                     \
  XX(33, SOURCE,      SOURCE)       \

/* Status Codes */
#define HTTP_STATUS_MAP(XX)                                                 \
  XX(100, CONTINUE,                        Continue)                        \
  XX(101, SWITCHING_PROTOCOLS,             Switching Protocols)             \
  XX(102, PROCESSING,                      Processing)                      \
  XX(200, OK,                              OK)                              \
  XX(201, CREATED,                         Created)                         \
  XX(202, ACCEPTED,                        Accepted)                        \
  XX(203, NON_AUTHORITATIVE_INFORMATION,   Non-Authoritative Information)   \
  XX(204, NO_CONTENT,                      No Content)                      \
  XX(205, RESET_CONTENT,                   Reset Content)                   \
  XX(206, PARTIAL_CONTENT,                 Partial Content)                 \
  XX(207, MULTI_STATUS,                    Multi-Status)                    \
  XX(208, ALREADY_REPORTED,                Already Reported)                \
  XX(226, IM_USED,                         IM Used)                         \
  XX(300, MULTIPLE_CHOICES,                Multiple Choices)                \
  XX(301, MOVED_PERMANENTLY,               Moved Permanently)               \
  XX(302, FOUND,                           Found)                           \
  XX(303, SEE_OTHER,                       See Other)                       \
  XX(304, NOT_MODIFIED,                    Not Modified)                    \
  XX(305, USE_PROXY,                       Use Proxy)                       \
  XX(307, TEMPORARY_REDIRECT,              Temporary Redirect)              \
  XX(308, PERMANENT_REDIRECT,              Permanent Redirect)              \
  XX(400, BAD_REQUEST,                     Bad Request)                     \
  XX(401, UNAUTHORIZED,                    Unauthorized)                    \
  XX(402, PAYMENT_REQUIRED,                Payment Required)                \
  XX(403, FORBIDDEN,                       Forbidden)                       \
  XX(404, NOT_FOUND,                       Not Found)                       \
  XX(405, METHOD_NOT_ALLOWED,              Method Not Allowed)              \
  XX(406, NOT_ACCEPTABLE,                  Not Acceptable)                  \
  XX(407, PROXY_AUTHENTICATION_REQUIRED,   Proxy Authentication Required)   \
  XX(408, REQUEST_TIMEOUT,                 Request Timeout)                 \
  XX(409, CONFLICT,                        Conflict)                        \
  XX(410, GONE,                            Gone)                            \
  XX(411, LENGTH_REQUIRED,                 Length Required)                 \
  XX(412, PRECONDITION_FAILED,             Precondition Failed)             \
  XX(413, PAYLOAD_TOO_LARGE,               Payload Too Large)               \
  XX(414, URI_TOO_LONG,                    URI Too Long)                    \
  XX(415, UNSUPPORTED_MEDIA_TYPE,          Unsupported Media Type)          \
  XX(416, RANGE_NOT_SATISFIABLE,           Range Not Satisfiable)           \
  XX(417, EXPECTATION_FAILED,              Expectation Failed)              \
  XX(421, MISDIRECTED_REQUEST,             Misdirected Request)             \
  XX(422, UNPROCESSABLE_ENTITY,            Unprocessable Entity)            \
  XX(423, LOCKED,                          Locked)                          \
  XX(424, FAILED_DEPENDENCY,               Failed Dependency)               \
  XX(426, UPGRADE_REQUIRED,                Upgrade Required)                \
  XX(428, PRECONDITION_REQUIRED,           Precondition Required)           \
  XX(429, TOO_MANY_REQUESTS,               Too Many Requests)               \
  XX(431, REQUEST_HEADER_FIELDS_TOO_LARGE, Request Header Fields Too Large) \
  XX(451, UNAVAILABLE_FOR_LEGAL_REASONS,   Unavailable For Legal Reasons)   \
  XX(500, INTERNAL_SERVER_ERROR,           Internal Server Error)           \
  XX(501, NOT_IMPLEMENTED,                 Not Implemented)                 \
  XX(502, BAD_GATEWAY,                     Bad Gateway)                     \
  XX(503, SERVICE_UNAVAILABLE,             Service Unavailable)             \
  XX(504, GATEWAY_TIMEOUT,                 Gateway Timeout)                 \
  XX(505, HTTP_VERSION_NOT_SUPPORTED,      HTTP Version Not Supported)      \
  XX(506, VARIANT_ALSO_NEGOTIATES,         Variant Also Negotiates)         \
  XX(507, INSUFFICIENT_STORAGE,            Insufficient Storage)            \
  XX(508, LOOP_DETECTED,                   Loop Detected)                   \
  XX(510, NOT_EXTENDED,                    Not Extended)                    \
  XX(511, NETWORK_AUTHENTICATION_REQUIRED, Network Authentication Required) \

/**
 * @brief HTTP方法枚举
 */
enum class HttpMethod {
#define XX(num, name, string) name = num,
    HTTP_METHOD_MAP(XX)     // HTTP_METHOD_MAP(XX) -> XX(0,DELETE,DELETE) -> DELETE = 0
#undef XX
    INVALID_METHOD      // 不支持的方法
};

/// 根据string/char*获得Http方法
HttpMethod StringToHttpMethod(const std::string& m);
HttpMethod CharsToHttpMethod(const char* m);
/// 将http方法转换为字符串
const char* HttpMethodToString(const HttpMethod& m);

/**
 * @brief HTTP状态枚举
 */
enum class HttpStatus {
#define XX(code, name, desc) name = code,
    HTTP_STATUS_MAP(XX)
#undef XX
};
/// 将HTTP状态枚举转换成字符串
const char* HttpStatusToString(const HttpStatus& s);

struct CaseInsensitiveLess {
    bool operator() (const std::string& lhs, const std::string& rhs) const ;
}

class HttpRequest {
public:
    typedef std::shared_ptr<HttpRequest> ptr;
    typedef std::map<std::string, std::string, CaseInsensitiveLess> MapType;
    

    HttpRequest(uint8_t version = 0x11, bool close = true);     // 默认非长连接

    HttpMethod getMethod() const { return m_method;}
    uint8_t getVersion() const { return m_version;}
    const std::string& getPath() const { return m_path;}
    const std::string getQuery() const { return m_query;}
    const std::string& getBody() const { return m_body;}
    
    const MapType& getHeaders() const { return m_headers;}
    const MapType& getParams() const { return m_params;}
    const MapType& getCookies() const { return m_cookies;}

    // 查找当前header是否有这个定义，如果有则返回值，否则返回空
    std::string getHeader(const std::string& key, const std::string& def = "") const;
    std::string getParam(const std::string& key, const std::string& def = "") const;
    std::string getCookie(const std::string& key, const std::string& def = "") const;

    void setMethod(HttpMethod v) { m_method = v;}
    void setVersion(uint8_t v) { m_version = v;}
    void setPath(const std::string& v) { m_path = v;}
    void setQuery(const std::string& v) { m_query = v;}
    void setFragment(const std::string& v) { m_fragment = v;}
    void setBody(const std::string& v) { m_body = v;}
    
    void setHeaders(const MapType& v) { m_headers = v;}
    void setParams(const MapType& v) { m_params = v;}
    void setCookies(const MapType& v) { m_cookies = v;}

    void setHeader(const std:string& key, const std::string& val);
    void setParam(const std:string& key, const std::string& val);
    void setCookie(const std:string& key, const std::string& val);
    
    void delHeader(const std::string& key);
    void delParam(const std::string& key);
    void delCookie(const std::string& key);
    
    /// 判断HTTP请求的头部参数是否存在,如果存在,val非空则赋值
    bool hasHeader(const std::string& key, std::string* val = nullptr);
    bool hasParam(const std::string& key, std::string* val = nullptr);
    bool hasParam(const std::string& key, std::string* val = nullptr);
   
private:
    //获取Map中的key值,并转成对应类型,返回是否成功
    template<class T>
    bool getAs(const MapType& m, const std::string& key, T& val, const T& def = T()) {
        auto it = m.find(key);
        if (it == m.end()) {
            val = def;
            return false;
        }
        try {
            val = boost::lexical_cast<T>(it->second);
            return true;
        } catch (...) {
            val = def;
        }
        return false
    }
    template<class MapType, class T>
    T getAs(const MapType& m, const std::string& key, const T& def = T()) {
        auto it = m.find(key);
        if(it == m.end()) {
            return def;
        }
        try {
            return boost::lexical_cast<T>(it->second);
        } catch (...) {
        }
        return def;
    }
    
private:
    HttpMethod m_method;
    uint8_t m_version;  // 版本 1.0/1.1/2.0 -> 0x00/0x01/0x10
    bool m_close;       // 是否自动关闭
    
    /// url相关 http://www.baidu.com:80/page/xxx?id=10&v=20#fr
    /// fr -> fragment  fragment 对于 HTML 文档来说就是页面内的定位标识符，可以实现 HTML 页面内的定位
    /// https://juejin.cn/post/6864802520024399879#heading-4
    std::string m_path;     // 请求路径
    std::string m_query;    // 请求参数
    std::string m_fragment; // 请求fragment
    
    // 消息头
    MapType m_headers; // 请求头部 
    MapType m_params;  // 请求参数
    MapType m_cookies; // 请求cookie

    // 消息体
    std::string m_body;
}

}
}

#endif