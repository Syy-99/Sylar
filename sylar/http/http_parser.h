/**
 * @file http_parser.h
 * @brief HTTP协议解析封装
 */

#ifndef __SYLAR_HTTP_PARSER_H__
#define __SYLAR_HTTP_PARSER_H__

#include "http.h"
#include "http11_parser.h"
#include "httpclient_parser.h"

namespace sylar {
namespace http {

class HttpRequestParser {
public:
    typedef std::shared_ptr<HttpRequestParser> ptr;
    HttpRequestParser();

    size_t execute(char *data, size_t len);
    int isFinished();
    int hasError();

    HttpRequest::ptr getData() const { return m_data;}

    void setError(int v) { m_error = v;}

    uint64_t getContentLength();
    
public:
    /**
     * @brief 返回HttpRequest协议解析的缓存大小
     */
    static uint64_t GetHttpRequestBufferSize();

    /**
     * @brief 返回HttpRequest协议的最大消息体大小
     */
    static uint64_t GetHttpRequestMaxBodySize();

    const http_parser& getParser() const { return m_parser;}
    
private:
    /// http_parser
    http_parser m_parser;
    /// HttpRequest结构：报文解析后需要放置到我们的结构中
    HttpRequest::ptr m_data;

    /// 错误码
    /// 1001: invalid version
    /// 1002: invalid field
    int m_error;

};


class HttpResponseParser {
public:
    typedef std::shared_ptr<HttpResponseParser> ptr;
    HttpResponseParser();

    /**
     * @brief 解析HTTP响应协议
     * @param[in, out] data 协议数据内存
     * @param[in] len 协议数据内存大小
     * @param[in] chunck 是否在解析chunck
     * @return 返回实际解析的长度,并且移除已解析的数据
     */
    size_t execute(char* data, size_t len, bool chunck);
    int isFinished();
    int hasError();

    HttpResponse::ptr getData() const { return m_data;}

    void setError(int v) { m_error = v;}

    uint64_t getContentLength();

public:

/// 返回HTTP响应解析缓存大小
static uint64_t GetHttpResponseBufferSize();
/// 返回HTTP响应最大消息体大小
static uint64_t GetHttpResponseMaxBodySize();

const httpclient_parser& getParser() const { return m_parser;}

private:
    /// http_parser
    httpclient_parser m_parser;
    /// HttpRequest结构：报文解析后需要放置到我们的结构中
    HttpResponse::ptr m_data;

    /// 错误码
    /// 1000: invalid method
    /// 1001: invalid version
    /// 1002: invalid field
    int m_error;

};

}
}

#endif
