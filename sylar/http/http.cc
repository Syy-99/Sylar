#include "http.h"

namespace sylar {
namespace http {
/// 根据string/char*获得Http方法
HttpMethod StringToHttpMethod(const std::string& m) {
#define XX(num, name, string) \
    if(strcmp(#string, m.c_str()) == 0) { \
        return HttpMethod::name; \
    }
    // 上面为什么用#string而不是#name? 难道这个m是从HTTP报文中获得的描述吗???
    HTTP_METHOD_MAP(XX);
#undef XX
    return HttpMethod::INVALID_METHOD;
}

HttpMethod CharsToHttpMethod(const char* m) {
#define XX(num, name, string) \
    if(strncmp(#string, m, strlen(#string)) == 0) { \
        return HttpMethod::name; \
    }
    /// 这里应该可以修改成和下面一样使用switch(暂时不改了)
    HTTP_METHOD_MAP(XX);
#undef XX
    return HttpMethod::INVALID_METHOD;
}

static const char* s_method_string[] = {
#define XX(num, name, string) #string,
    HTTP_METHOD_MAP(XX)
#undef XX
};

/// 将http方法转换为字符串
const char* HttpMethodToString(const HttpMethod& m) {
    uint32_t idx = (uint32_t)m;     // HttpMethod实际是按0,1...这种数字
    if(idx >= (sizeof(s_method_string) / sizeof(s_method_string[0]))) { // 比数组大
        return "<unknown>";
    }
    return s_method_string[idx];
}

const char* HttpStatusToString(const HttpStatus& s) {
    switch(s) {
#define XX(code, name, msg) \
        case HttpStatus::name: \
            return #msg;    // 注意这里返回的是message
        HTTP_STATUS_MAP(XX);
#undef XX
        default:
            return "<unknown>";
    }
}    

bool CaseInsensitiveLess::operator() (const std::string& lhs
                                     ,const std::string& rhs) const {
    return strcasecmp(lhs.c_str(), rhs.c_str()) < 0;  // 忽略大小写的比较
}

HttpRequest::HttpRequest(uint8_t version, bool close)
    :m_method(HttpMethod::GET)      // 默认GET方法
    ,m_version(version)             // 默认0x11,1.1
    ,m_close(close)
    ,m_path("/") {                 // 默认true, 非长连接
    
}

std::string HttpRequest::getHeader(const std::string& key, 
                                   const std::string& def) const {
    auto it = m_headers.find(key);
    return it == m_headers.end() ? def : it->second;
}
std::string HttpRequest::getParam(const std::string& key, 
                                  const std::string& def) const {
    auto it = m_params.find(key);
    return it == m_params.end() ? def : it->second;
}
std::string HttpRequest::getCookie(const std::string& key, 
                                   const std::string& def) const {
    auto it = m_cookies.find(key);
    return it == m_cookies.end() ? def : it->second;
}

void HttpRequest::setHeader(const std::string& key, const std::string& val) {
    m_headers[key] = val;
}
void HttpRequest::setParam(const std::string& key, const std::string& val) {
    m_params[key] = val;
}
void HttpRequest::setCookie(const std::string& key, const std::string& val) {
    m_cookies[key] = val;
}

void HttpRequest::delHeader(const std::string& key) {
    m_headers.erase(key);
}
void HttpRequest::delParam(const std::string& key) {
    m_params.erase(key);
}
void HttpRequest::delCookie(const std::string& key) {
    m_params.erase(key);
}

/// 判断HTTP请求的头部参数是否存在,如果存在,val非空则赋值
bool HttpRequest::hasHeader(const std::string& key, std::string* val) {
    auto it = m_headers.find(key);
    if (it == m_headers.end()) {
        return false;
    }
    if(val) {
        *val = it->second;
    }
    return true;
}
bool HttpRequest::hasParam(const std::string& key, std::string* val) {
    auto it = m_params.find(key);
    if(it == m_params.end()) {
        return false;
    }
    if(val) {
        *val = it->second;
    }
    return true;
}
bool HttpRequest::hasCookie(const std::string& key, std::string* val) {
    auto it = m_cookies.find(key);
    if(it == m_cookies.end()) {
        return false;
    }
    if(val) {
        *val = it->second;
    }
    return true;
}

std::ostream& HttpRequest::dump(std::ostream& os) const {
    // GET /uri HTTP/1.1
    // Host: www.baidu.com
    //..
    //空行
    //消息体

    // 请求行
    os << HttpMethodToString(m_method) << " "
       << m_path
       << (m_query.empty() ? "" : "?")
       << m_query
       << (m_fragment.empty() ? "" : "#")
       << m_fragment
       << " HTTP/"
       << ((uint32_t)(m_version >> 4))
       << "."
       << ((uint32_t)(m_version & 0x0F))
       << "\r\n";
    
    // 消息头
    for(auto& i : m_headers) {
        if (strcasecmp(i.first.c_str(), "connection") == 0) {
            continue;
        }
        os << i.first << ": " << i.second << "\r\n";
    }
    os << "connection: " << (m_close ? "close" : "keep-alive") << "\r\n";


    // 消息体
    if(!m_body.empty()) {
        // 如果有body就需要长度，来避免粘包
        os << "content-length: " << m_body.size() << "\r\n\r\n"
           << m_body;
    } else {
        os << "\r\n";
    }
    return os;
}


std::string HttpRequest::toString() const {
    std::stringstream ss;
    dump(ss);
    return ss.str();
}


HttpResponse::HttpResponse(uint8_t version, bool close)
    :m_status(HttpStatus::OK)
    ,m_version(version)
    ,m_close(close) {
}


std::string HttpResponse::getHeader(const std::string& key, const std::string& def) const {
    auto it = m_headers.find(key);
    return it == m_headers.end() ? def : it->second;
}

void HttpResponse::setHeader(const std::string& key, const std::string& val) {
    m_headers[key] = val;
}

void HttpResponse::delHeader(const std::string& key) {
    m_headers.erase(key);
}

std::ostream& HttpResponse::dump(std::ostream& os) const {
    // HTTP/1.1 200 OK
    // 消息头
    //
    // 消息体

    os << "HTTP/"
       << ((uint32_t)(m_version >> 4))
       << "."
       << ((uint32_t)(m_version & 0x0F))
       << " "
       << (uint32_t)m_status
       << " "
       << (m_reason.empty() ? HttpStatusToString(m_status) : m_reason)
       << "\r\n";

    for(auto& i : m_headers) {
        if (strcasecmp(i.first.c_str(), "connection") == 0) {
            continue;
        }
        os << i.first << ": " << i.second << "\r\n";
    }
    // connection由m_close决定
    os << "connection: " << (m_close ? "close" : "keep-alive") << "\r\n";

    if(!m_body.empty()) {
        os << "content-length: " << m_body.size() << "\r\n\r\n"
           << m_body;
    } else {
        os << "\r\n";
    }

    return os;
}

std::string HttpResponse::toString() const {
    std::stringstream ss;
    dump(ss);
    return ss.str();
}

std::ostream& operator<<(std::ostream& os, const HttpRequest& req) {
    return req.dump(os);
}

std::ostream& operator<<(std::ostream& os, const HttpResponse& rsp) {
    return rsp.dump(os);
}


}
}