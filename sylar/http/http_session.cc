
#include "http_session.h"
#include "http_parser.h"

#include "../log.h"

namespace sylar {
namespace http {

static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");
HttpSession::HttpSession(Socket::ptr sock, bool owner)
    :SocketStream(sock, owner) {        // 委托构造函数
}

/// 接收HTTP请求
HttpRequest::ptr HttpSession::recvRequest() {
    SYLAR_LOG_INFO(g_logger) <<"recvRequest begin ";
    HttpRequestParser::ptr parser(new HttpRequestParser); 

    // 获得设置的读缓冲大小并初始化缓冲区
    uint64_t buff_size = HttpRequestParser::GetHttpRequestBufferSize();  

    // 模拟小buffer
    // uint64_t buff_size = 100;     // 会出错, 截断了
    // 在on_request_http_field()中，
    // 读取User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/114.0.0.0 Safari/537.36
    // 时错误

    /*
    理解request的缓冲大小: 100 的问题
    第1次读：
    GET / HTTP/1.1
    Host: 10.249.44.138:8020
    Connection: keep-alive
    Cache-Control: max-age=0
    Upgrade-
    然后只能解析出来Host Connection Cache-Control， Upgrade-无法解析，但是nparse仍然=100, data又从0开始保存数据
    第2次读：
    Insecure-Requests: 1\r\n
    User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTM
    解析出来Insecure-Requests(本来就有问题)， User-Agent:无法解析，但是nparse仍然=100, data又从0开始保存数据
    第3次读：
    L, like Gecko) Chrome/114.0.0.0 Safari/537.36\r\n   -> 这一行解析不出来一个field，所以解析错误，但是解析的长度还是100
    Accept: text/html,application/xhtml+xml,application/x
    */
    std::shared_ptr<char> buffer(new char[buff_size], [](char* ptr){
                delete[] ptr;
    });
    char* data = buffer.get();

    // 开始读数据
    int offset = 0;
    do {
        int len = read(data + offset, buff_size - offset);
        if(len <= 0) {
            SYLAR_LOG_INFO(g_logger) <<"len error";
            // 发生错误，则关闭连接并返回
            close();
            return nullptr;
        } 

        // 解析HTTP报文
        len += offset;
        size_t nparse = parser->execute(data,len);
        SYLAR_LOG_INFO(g_logger) << std::endl << data;
        SYLAR_LOG_INFO(g_logger) << std::endl << offset << " - " << len << " - "<< nparse;
        if(parser->hasError()) {  
            SYLAR_LOG_INFO(g_logger) <<"parser error " << nparse;
            // 如果解析出错，则关闭连接
            close();
            return nullptr;
        }
        offset = len - nparse;  // ??? 有问题，逻辑上不太对????
        if(offset == (int)buff_size) {      // ??? 这里怎么解析的
             SYLAR_LOG_INFO(g_logger) <<"buffer size too small" << offset;
            // 缓冲区满了都没有解析完成（请求行+消息头）， 则认为这个HTTP请求是非法的
            close();
            return nullptr;
        }

        if(parser->isFinished()) {
            break;  // 解析完成，则跳出循环
        }
        // 还没有解析完，则继续while循环读
    }while(true);
    // 到这里，说明解析成功

    int64_t length = parser->getContentLength();
    if(length > 0) {
        std::string body;
        body.resize(length);

        int len = 0;
        if(length >= offset) {
            memcpy(&body[0], data, offset);     // 先从data中获得body的内容
            len = offset;
        } else {
            memcpy(&body[0], data, length);
            len = length;
        }
        length -= offset;

        if(length > 0) {    // 说明还有body没发过来，则需要继续读
            if(readFixSize(&body[len], length) <= 0) {
                close();
                return nullptr;
            }
        }
        parser->getData()->setBody(body);
    }

    // parser->getData()->init();
    return parser->getData();
}

/// 发送HTTP响应
int HttpSession::sendResponse(HttpResponse::ptr rsp) {
    std::stringstream ss;
    ss << *rsp;
    std::string data = ss.str();
    
    return writeFixSize(data.c_str(), data.size());
}


}
}