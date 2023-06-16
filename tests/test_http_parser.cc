#include "../sylar/http/http_parser.h"
#include "../sylar/log.h"

static sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

const char test_request_data[] = "GET / HTTP/1.1\r\n"
                                "Host: www.sylar.top\r\n"
                                "Content-Length: 10\r\n\r\n"
                                "1234567890";
// 解析这个，execute会在\r\n出结束，返回60字节(包括\r\n)

// const char test_request_data[] = "GET / HTTP/3.1\r\n"
//                                 "Host: www.sylar.top\r\n"
//                                 "Content-Length: 10\r\n\r\n"
//                                 "1234567890";
// 如果解析这里，execute应该是解析到3哪里就发现错误，返回11个字节

// const char test_request_data[] = "GET / HTTP/1.1\r\n"
//                                 "Host: www.sylar.top\r\n"
//                                 "Content-Length: 10\r\n";
// 如果解析这个，parser.isFinished()=0, 表示还没有结束


// const char test_request_data[] = "GET / HTTP/1.1\r\n"
//                                 "Host: www.sylar.top......(非常长).....\r\n"
//                                 "Content-Length: 10\r\n";
void test_request() {
    sylar::http::HttpRequestParser parser;
    std::string tmp = test_request_data;

    // 解析HTTP请求报文
    size_t s = parser.execute(&tmp[0], tmp.size());
    // 这里解析时，应该是根据空行\r\n来结束解析，所以s会返回目前已经解析的字节
    SYLAR_LOG_ERROR(g_logger) << "execute rt=" << s
        << " has_error=" << parser.hasError()         // 是否解析错误    
        << " is_finished=" << parser.isFinished()   // 是否完整解析了一个HTTP请求报文
        << " total=" << tmp.size()  
        << " content_length=" << parser.getContentLength();    // HTTP请求报文中的消息体的长度

    tmp.resize(tmp.size() - s);     // 删除之前被移到后面去的
    SYLAR_LOG_INFO(g_logger) << parser.getData()->toString();
    SYLAR_LOG_INFO(g_logger) << tmp;    // 获得后面可能的body部分
}   


const char test_response_data[] = "HTTP/1.1 200 OK\r\n"
        "Date: Tue, 04 Jun 2019 15:43:56 GMT\r\n"
        "Server: Apache\r\n"
        "Last-Modified: Tue, 12 Jan 2010 13:48:00 GMT\r\n"
        "ETag: \"51-47cf7e6ee8400\"\r\n"
        "Accept-Ranges: bytes\r\n"
        "Content-Length: 81\r\n"
        "Cache-Control: max-age=86400\r\n"
        "Expires: Wed, 05 Jun 2019 15:43:56 GMT\r\n"
        "Connection: Close\r\n"
        "Content-Type: text/html\r\n\r\n"
        "<html>\r\n"
        "<meta http-equiv=\"refresh\" content=\"0;url=http://www.baidu.com/\">\r\n"
        "</html>\r\n";
void test_response() {
    sylar::http::HttpResponseParser parser;
    std::string tmp = test_response_data;
    size_t s = parser.execute(&tmp[0], tmp.size());
    SYLAR_LOG_ERROR(g_logger) << "execute rt=" << s
        << " has_error=" << parser.hasError()
        << " is_finished=" << parser.isFinished()
        << " total=" << tmp.size()
        << " content_length=" << parser.getContentLength()
        << " tmp=" << tmp;        // 查看被移动后的tmp的样式

    tmp.resize(tmp.size() - s);

    SYLAR_LOG_INFO(g_logger) << parser.getData()->toString();
    SYLAR_LOG_INFO(g_logger) << tmp;
}
int main() {
    // test_request();
    test_response();
    return 0;
}