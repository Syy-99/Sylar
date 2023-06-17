#include "http_parser.h"
#include "../log.h"
#include "../config.h"
#include <string.h>

namespace sylar {
namespace http {

static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

// 设置HTTP的缓存大小
/// g_http_request_buffer_size实际是限制（请求行+消息头）的大小，因为http_parser将它和消息体分开的
static sylar::ConfigVar<uint64_t>::ptr g_http_request_buffer_size =
    sylar::Config::Lookup("http.request.buffer_size"
                ,(uint64_t)(4 * 1024), "http request buffer size");

// 请求的消息体部分如果很大，就认为是非法的， 64M
static sylar::ConfigVar<uint64_t>::ptr g_http_request_max_body_size =
    sylar::Config::Lookup("http.request.max_body_size"
                ,(uint64_t)(64 * 1024 * 1024), "http request max body size");


static sylar::ConfigVar<uint64_t>::ptr g_http_response_buffer_size =
    sylar::Config::Lookup("http.response.buffer_size"
                ,(uint64_t)(4 * 1024), "http response buffer size");

static sylar::ConfigVar<uint64_t>::ptr g_http_response_max_body_size =
    sylar::Config::Lookup("http.response.max_body_size"
                ,(uint64_t)(64 * 1024 * 1024), "http response max body size");

// 记录  在哪里使用??? -> http_sessiont中使用，用来存放HTTP请求
static uint64_t s_http_request_buffer_size = 0;
static uint64_t s_http_request_max_body_size = 0;

static uint64_t s_http_response_buffer_size = 0;    // ??? 这个没有用上把????
static uint64_t s_http_response_max_body_size = 0;

uint64_t HttpRequestParser::GetHttpRequestBufferSize() {
    return s_http_request_buffer_size;
}

uint64_t HttpRequestParser::GetHttpRequestMaxBodySize() {
    return s_http_request_max_body_size;
}

uint64_t HttpResponseParser::GetHttpResponseBufferSize() {
    return s_http_response_buffer_size;
}

uint64_t HttpResponseParser::GetHttpResponseMaxBodySize() {
    return s_http_response_max_body_size;
}

namespace {
struct _RequestSizeIniter {     // 做变量初始化
      _RequestSizeIniter() {
        s_http_request_buffer_size = g_http_request_buffer_size->getValue();
        s_http_request_max_body_size = g_http_request_max_body_size->getValue();
        s_http_response_buffer_size = g_http_response_buffer_size->getValue();
        s_http_response_max_body_size = g_http_response_max_body_size->getValue();

        // 设置配置变更的回调函数， 重新设置配置的值
        g_http_request_buffer_size->addListener(
                [](const uint64_t& ov, const uint64_t& nv){
                s_http_request_buffer_size = nv;
        });

        g_http_request_max_body_size->addListener(
                [](const uint64_t& ov, const uint64_t& nv){
                s_http_request_max_body_size = nv;
        });

        g_http_response_buffer_size->addListener(
                [](const uint64_t& ov, const uint64_t& nv){
                s_http_response_buffer_size = nv;
        });

        g_http_response_max_body_size->addListener(
                [](const uint64_t& ov, const uint64_t& nv){
                s_http_response_max_body_size = nv;
        });

      }

};
static _RequestSizeIniter _init;
}

void on_request_method(void *data, const char *at, size_t length) {
    // 解析出method就会回调该函数，让我们处理
    HttpRequestParser* parser = static_cast<HttpRequestParser*>(data);

    // at中指向的是保存HTTP Method方法的字符串
    HttpMethod m = CharsToHttpMethod(at);

    if(m == HttpMethod::INVALID_METHOD) {
        SYLAR_LOG_WARN(g_logger) << "invalid http request method: "
            << std::string(at, length);
        parser->setError(1000);     // 设置错误码
        return;
    }

    parser->getData()->setMethod(m);    // 设置HttpRequest中的Method字段

}
void on_request_uri(void *data, const char *at, size_t length) {
    /// 不需要做，因为我们的HttpRequest是分成path, query, fragment来存储的
}
void on_request_fragment(void *data, const char *at, size_t length) {
    HttpRequestParser* parser = static_cast<HttpRequestParser*>(data);

    // void setFragment(const std::string& v);
    parser->getData()->setFragment(std::string(at, length));
}
void on_request_path(void *data, const char *at, size_t length) {
    HttpRequestParser* parser = static_cast<HttpRequestParser*>(data);
    parser->getData()->setPath(std::string(at, length));
}
void on_request_query(void *data, const char *at, size_t length) {
    HttpRequestParser* parser = static_cast<HttpRequestParser*>(data);
    parser->getData()->setQuery(std::string(at, length));
}
void on_request_version(void *data, const char *at, size_t length) {
    HttpRequestParser* parser = static_cast<HttpRequestParser*>(data);
    /// 我们的version是uin8类型
    uint8_t v = 0;
    if(strncmp(at, "HTTP/1.1", length) == 0) {
        v = 0x11;
    } else if(strncmp(at, "HTTP/1.0", length) == 0) {
        v = 0x10;
    } else {
        SYLAR_LOG_WARN(g_logger) << "invalid http request version: "
            << std::string(at, length);
        parser->setError(1001);
        return;
    }
    parser->getData()->setVersion(v);
}
void on_request_header_done(void *data, const char *at, size_t length) {
    // HttpRequestParser* parser = static_cast<HttpRequestParser*>(data);
    // ????
}


void on_request_http_field(void *data, const char *field, size_t flen
                           ,const char *value, size_t vlen) {
    /// 每解析出一个头部字段，就会调用这个函数

    HttpRequestParser* parser = static_cast<HttpRequestParser*>(data);

    if(flen == 0) {
        SYLAR_LOG_WARN(g_logger) << "invalid http request field length == 0" << field;
        // parser->setError(1002);
        return;
    }
    SYLAR_LOG_WARN(g_logger) << "filed " << std::string(field, flen) << " " << std::string(value, vlen);
    parser->getData()->setHeader(std::string(field, flen)
                                ,std::string(value, vlen));
}
HttpRequestParser::HttpRequestParser() : m_error(0) {
    m_data.reset(new sylar::http::HttpRequest);

    http_parser_init(&m_parser);
    // 设置回调函数
    m_parser.request_method = on_request_method;
    m_parser.request_uri = on_request_uri;
    m_parser.fragment = on_request_fragment;
    m_parser.request_path = on_request_path;
    m_parser.query_string = on_request_query;
    m_parser.http_version = on_request_version;
    m_parser.header_done = on_request_header_done;
    m_parser.http_field = on_request_http_field;
    
    m_parser.data = this;       // 设置this, 使得回调的时候可以通过data拿到HttpRequestParser类对象
    
}

//-1: 执行有错误
//>0: 已处理的字节数，且data有效数据为len -这个值;????（说明HTTP报文还没有解析完) 
// 为什么会出现这种情况??

/// ??? 怎么用
size_t HttpRequestParser::execute(char *data, size_t len) {
    /// execute可能需要多次执行才能解析到一个完整的TTP报文
    /// 每次解析需要返回一个状态，来判断是否需要继续解析（可以参考TinyWebServer里的有限状态机实现）
    
    // data应该是接受到的HTTP请求报文的部分
    size_t offset = http_parser_execute(&m_parser, data, len, 0);   // 0 data的开头始终是未处理的
    
    memmove(data, data + offset, (len - offset));   // 将解析过的内存移走，腾出缓存空间
    
    return offset;
}
int HttpRequestParser::isFinished() {
    return http_parser_finish(&m_parser);
}
int HttpRequestParser::hasError() {
    // 解析过程可能有问题，解析出来的报文可以有问题
    return m_error || http_parser_has_error(&m_parser);  
}   

uint64_t HttpRequestParser::getContentLength() {
    return m_data->getHeaderAs<uint64_t>("Content-length", 0);
}


void on_response_reason(void *data, const char *at, size_t length) {
    HttpResponseParser* parser = static_cast<HttpResponseParser*>(data);
    parser->getData()->setReason(std::string(at, length));
}

void on_response_status(void *data, const char *at, size_t length) {
    HttpResponseParser* parser = static_cast<HttpResponseParser*>(data);
    HttpStatus status = (HttpStatus)(atoi(at));     
    parser->getData()->setStatus(status);    
}

void on_response_chunk(void *data, const char *at, size_t length) {
    
}

void on_response_version(void *data, const char *at, size_t length) {
    HttpResponseParser* parser = static_cast<HttpResponseParser*>(data);
    uint8_t v = 0;
    if(strncmp(at, "HTTP/1.1", length) == 0) {
        v = 0x11;
    } else if(strncmp(at, "HTTP/1.0", length) == 0) {
        v = 0x10;
    } else {
        SYLAR_LOG_WARN(g_logger) << "invalid http response version: "
            << std::string(at, length);
        parser->setError(1001);
        return;
    }

    parser->getData()->setVersion(v);    
}

void on_response_header_done(void *data, const char *at, size_t length) {
    
}

void on_response_last_chunk(void *data, const char *at, size_t length) {
    
}

void on_response_http_field(void *data, const char *field, size_t flen
                           ,const char *value, size_t vlen) {
    HttpResponseParser* parser = static_cast<HttpResponseParser*>(data);
    if(flen == 0) {
        SYLAR_LOG_WARN(g_logger) << "invalid http response field length == 0";
        // parser->setError(1002); 这里先不设置，否则无法返回response
        return;
    }
    parser->getData()->setHeader(std::string(field, flen)
                                ,std::string(value, vlen));
}

HttpResponseParser::HttpResponseParser() : m_error(0){
    m_data.reset(new sylar::http::HttpResponse);

    httpclient_parser_init(&m_parser);
    // 设置回调函数
    m_parser.reason_phrase = on_response_reason;
    m_parser.status_code = on_response_status;
    m_parser.chunk_size = on_response_chunk;
    m_parser.http_version = on_response_version;
    m_parser.header_done = on_response_header_done;
    m_parser.last_chunk = on_response_last_chunk;
    m_parser.http_field = on_response_http_field;

    m_parser.data = this; 
}

size_t HttpResponseParser::execute(char* data, size_t len, bool chunck) {
    if(chunck) {
        httpclient_parser_init(&m_parser);      // 重新解析
    }

    size_t offset = httpclient_parser_execute(&m_parser, data, len, 0);
    memmove(data, data + offset, (len - offset));   // 将处理过的数据移走，剩下的数据放在缓存的前面

    return offset;
}
int HttpResponseParser::isFinished() {
    return httpclient_parser_finish(&m_parser);
}
int HttpResponseParser::hasError() {
    return m_error || httpclient_parser_has_error(&m_parser);
}

uint64_t HttpResponseParser::getContentLength() {
    return m_data->getHeaderAs<uint64_t>("content-length", 0);
}


}
}