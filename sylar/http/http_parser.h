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

    size_t execute(char *data, size_t len);
    int isFinished();
    int hasError();

    HttpResponse::ptr getData() const { return m_data;}

    void setError(int v) { m_error = v;}

    uint64_t getContentLength();
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
