#include "http_connection.h"
#include "../log.h"
#include "http_parser.h"

namespace sylar {
namespace http {

static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

HttpResult::ptr HttpConnection::DoRequest(HttpMethod method
                , const std::string& url
                , uint64_t timeout_ms       // 表示超时
                , const std::map<std::string, std::string>& headers
                , const std::string& body) {
    Uri::ptr uri = Uri::Create(url);
    if(!uri) {
        return std::make_shared<HttpResult>((int)HttpResult::Error::INVALID_URL
                , nullptr, "invalid url: " + url);
    }
    return DoRequest(method, uri, timeout_ms, headers, body);
}

HttpResult::ptr HttpConnection::DoRequest(HttpMethod method
                , Uri::ptr uri
                , uint64_t timeout_ms
                , const std::map<std::string, std::string>& headers
                , const std::string& body) {
    // 最终实现

    // 构造HTTP请求报文
    HttpRequest::ptr req = std::make_shared<HttpRequest>();
    req->setPath(uri->getPath());
    req->setQuery(uri->getQuery());
    req->setFragment(uri->getFragment());
    req->setMethod(method);

    bool has_host = false;
    for(auto& i : headers) {
         if(strcasecmp(i.first.c_str(), "connection") == 0) {
            if(strcasecmp(i.second.c_str(), "keep-alive") == 0) {
                req->setClose(false);       // 给服务器设置长连接
                // 设不设置keep-live对连接有什么影响??? 在代码上的处理区别体现在哪里???
            }
            continue;
         }

        if(!has_host && strcasecmp(i.first.c_str(), "host") == 0) {
            has_host = !i.second.empty();       // 确定有host字段
        }

        req->setHeader(i.first, i.second);
    }

    if(!has_host) {
        // 默认给host
        req->setHeader("Host", uri->getHost());
    }

    req->setBody(body);

    return DoRequest(req, uri, timeout_ms);
}

HttpResult::ptr HttpConnection::DoRequest(HttpRequest::ptr req
                , Uri::ptr uri
                , uint64_t timeout_ms) {
    // 这个函数基本就是封装了一个整体的流程
    Address::ptr addr = uri->createAddress();       // 从uri中获得socket地址
    if(!addr) {
        return std::make_shared<HttpResult>((int)HttpResult::Error::INVALID_HOST
                , nullptr, "invalid host: " + uri->getHost());
    }

    Socket::ptr sock =  Socket::CreateTCP(addr);    // 根据地址创建套接字
    if(!sock) {
        return std::make_shared<HttpResult>((int)HttpResult::Error::CREATE_SOCKET_ERROR
                , nullptr, "create socket fail: " + addr->toString()
                        + " errno=" + std::to_string(errno)
                        + " errstr=" + std::string(strerror(errno)));
    }

    // 开始连接
    if(!sock->connect(addr)) {
        return std::make_shared<HttpResult>((int)HttpResult::Error::CONNECT_FAIL
                , nullptr, "connect fail: " + addr->toString());
    }

    // 发送HTTP报文
    sock->setRecvTimeout(timeout_ms);   // 设置超时时间 ??? 这个超时是针对哪些方法的??

    // 创建HttpConnection
    HttpConnection::ptr conn = std::make_shared<HttpConnection>(sock);
    int rt = conn->sendRequest(req);    // 通过HttpConnection发送请求
    if(rt == 0) {
        return std::make_shared<HttpResult>((int)HttpResult::Error::SEND_CLOSE_BY_PEER
                , nullptr, "send request closed by peer: " + addr->toString());
    }
    if(rt < 0) {
        return std::make_shared<HttpResult>((int)HttpResult::Error::SEND_SOCKET_ERROR
                    , nullptr, "send request socket error errno=" + std::to_string(errno)
                    + " errstr=" + std::string(strerror(errno)));
    }

    auto rsp = conn->recvResponse();
    if(!rsp) {
        return std::make_shared<HttpResult>((int)HttpResult::Error::TIMEOUT
                    , nullptr, "recv response timeout: " + addr->toString()
                    + " timeout_ms:" + std::to_string(timeout_ms));
    }

    // 将rsp返回
    return std::make_shared<HttpResult>((int)HttpResult::Error::OK, rsp, "ok");
}

HttpResult::ptr HttpConnection::DoGet(const std::string& url
                , uint64_t timeout_ms
                , const std::map<std::string, std::string>& headers
                , const std::string& body) {
    Uri::ptr uri = Uri::Create(url);    // 解析url
    if (!uri) {
        return std::make_shared<HttpResult>((int)HttpResult::Error::INVALID_URL
                , nullptr, "invalid url: " + url);
    }

    return DoGet(uri, timeout_ms, headers, body);           
}

HttpResult::ptr HttpConnection::DoGet(Uri::ptr uri
                , uint64_t timeout_ms
                , const std::map<std::string, std::string>& headers
                , const std::string& body) {
    return DoRequest(HttpMethod::GET, uri, timeout_ms, headers, body);
}

HttpResult::ptr HttpConnection::DoPost(const std::string& url
                    , uint64_t timeout_ms
                    , const std::map<std::string, std::string>& headers 
                    , const std::string& body) {
    Uri::ptr uri = Uri::Create(url);    // 解析url
    if (!uri) {
        // HTTP请求的uri都有问题，则直接返回错误信息
        return std::make_shared<HttpResult>((int)HttpResult::Error::INVALID_URL
                , nullptr, "invalid url: " + url);
    }

    return DoPost(uri, timeout_ms, headers, body);  
}

HttpResult::ptr HttpConnection::DoPost(Uri::ptr uri
                    , uint64_t timeout_ms
                    , const std::map<std::string, std::string>& headers
                    , const std::string& body) {
    return DoRequest(HttpMethod::POST, uri, timeout_ms, headers, body);
}


HttpConnection::HttpConnection(Socket::ptr sock, bool owner)
    :SocketStream(sock, owner) {
}

/// 接收HTTP请求
// HttpResponse::ptr HttpConnection::recvResponse() {
//     SYLAR_LOG_INFO(g_logger) <<"recvRequest begin ";
//     HttpResponseParser::ptr parser(new HttpResponseParser); 

//     // 获得设置的读缓冲大小并初始化缓冲区
//     uint64_t buff_size = HttpRequestParser::GetHttpRequestBufferSize();  

//     // 模拟小buffer
//     // uint64_t buff_size = 100;     
//     std::shared_ptr<char> buffer(new char[buff_size], [](char* ptr){
//                 delete[] ptr;
//     });
//     char* data = buffer.get();

//     // 开始读数据
//     int offset = 0;
//     do {
//         int len = read(data + offset, buff_size - offset);
//         if(len <= 0) {
//             SYLAR_LOG_INFO(g_logger) <<"len error";
//             // 发生错误，则关闭连接并返回
//             close();
//             return nullptr;
//         } 

//         // 解析HTTP报文
//         len += offset;
//         data[len] = '\0';
//         size_t nparse = parser->execute(data,len, false);
//         // SYLAR_LOG_INFO(g_logger) << std::endl << data;
//         // SYLAR_LOG_INFO(g_logger) << std::endl << offset << " - " << len << " - "<< nparse;
//         if(parser->hasError()) {  
//             SYLAR_LOG_INFO(g_logger) <<"parser error " << nparse;
//             // 如果解析出错，则关闭连接
//             close();
//             return nullptr;
//         }
//         offset = len - nparse;  //  通常来说 nparse会等于len, 但是接受缓冲区接受到body时，nparse只会记录解析到底空行的位置，此时len!=npasre
//         if(offset == (int)buff_size) {    // 出现这个情况，只可能是缓冲区此时只有body但是还没有解析成功?? 怎么会有这种情况???
//              SYLAR_LOG_INFO(g_logger) <<"buffer size too small" << offset;
//             // 缓冲区满了都没有解析完成（请求行+消息头）， 则认为这个HTTP请求是非法的
//             close();
//             return nullptr;
//         }

//         if(parser->isFinished()) {
//             break;  // 解析完成，则跳出循环
//         }
//         // 还没有解析完，则继续while循环读
//     }while(true);
//     // 到这里，说明解析成功
//     auto& client_parser = parser->getParser();      
//      if(client_parser.chunked) {        // 如果是chunk Transfer-Encoding: chunked
//         //通常，HTTP应答消息中发送的数据是整个发送的，Content-Length消息头字段表示数据的长度。
//         //数据的长度很重要，因为客户端需要知道哪里是应答消息的结束，以及后续应答消息的开始。
//         //然而，使用分块传输编码，数据分解成一系列数据块，并以一个或多个块发送，
//         //这样服务器可以发送数据而不需要预先知道发送内容的总大小。

//         // 如果一个HTTP消息（请求消息或应答消息）的Transfer-Encoding消息头的值为chunked，
//         // 那么，消息体由数量未定的块组成，并以最后一个大小为0的块为结束。
//         std::string body;
//         do {
//             // ???逻辑不太懂
//             int len = offset;       // 记录body长度
//             do {
//                 int rt = read(data + len, buff_size - len);
//                 if(rt <= 0) {
//                     close();
//                     return nullptr;
//                 }
//                 len += rt;
//                 data[len] = '\0'; 

//                 size_t nparse = parser->execute(data, len, true);   // true分段解析
//                 if(parser->hasError()) {
//                     close();
//                     return nullptr;
//                 }
//                 len -= nparse;      // 剩余长度
//                 if(len == (int)buff_size) {     // data已经满了
//                     close();
//                     return nullptr;
//                 }
//             } while(!parser->isFinished());
//             len -= 2; // content_len的默认长度包括最后的r\n

//             SYLAR_LOG_DEBUG(g_logger) << "content_len=" << client_parser.content_len;
//             if (client_parser.content_len <= len) {
//                 body.append(data, client_parser.content_len);
//                 memmove(data, data + client_parser.content_len 
//                             , len - client_parser.content_len);     // 多余的数据移动到前面去
//             } else {
//                 body.append(data, len);
//                 int left = client_parser.content_len - len;

//                 while(left > 0) {
//                     int rt = read(data, left > (int)buff_size ? (int)buff_size : left);
//                     if(rt <= 0) {
//                         close();
//                         return nullptr;
//                     }
//                     body.append(data, rt);
//                     left -= rt;
//                 }
//                 len = 0;
//             }
//         } while(!client_parser.chunks_done);

//         // parser->getData()->setBody(body);
//      } else {
//         int64_t length = parser->getContentLength();
//         if(length > 0) {
//             std::string body;
//             body.resize(length);

//             int len = 0;
//             if(length >= offset) {
//                 memcpy(&body[0], data, offset);     // 先从data中获得body的内容
//                 len = offset;
//             } else {
//                 memcpy(&body[0], data, length);
//                 len = length;
//             }
//             length -= offset;

//             if(length > 0) {    // 说明还有body没发过来，则需要继续读
//                 if(readFixSize(&body[len], length) <= 0) {
//                     close();
//                     return nullptr;
//                 }
//             }
//             parser->getData()->setBody(body);
//         }
//      }

//     // parser->getData()->init();
//     return parser->getData();
// }
HttpResponse::ptr HttpConnection::recvResponse() {
    HttpResponseParser::ptr parser(new HttpResponseParser);
    uint64_t buff_size = HttpRequestParser::GetHttpRequestBufferSize();
    //uint64_t buff_size = 100;
    std::shared_ptr<char> buffer(
            new char[buff_size + 1], [](char* ptr){
                delete[] ptr;
            });
    char* data = buffer.get();
    int offset = 0;
    do {
        int len = read(data + offset, buff_size - offset);
        if(len <= 0) {
            close();
            return nullptr;
        }
        len += offset;
        data[len] = '\0';
        size_t nparse = parser->execute(data, len, false);
        if(parser->hasError()) {
            close();
            return nullptr;
        }
        offset = len - nparse;
        if(offset == (int)buff_size) {
            close();
            return nullptr;
        }
        if(parser->isFinished()) {
            break;
        }
    } while(true);
    auto& client_parser = parser->getParser();
    if(client_parser.chunked) {
        std::string body;
        int len = offset;
        do {
            do {
                int rt = read(data + len, buff_size - len);
                if(rt <= 0) {
                    close();
                    return nullptr;
                }
                len += rt;
                data[len] = '\0';
                size_t nparse = parser->execute(data, len, true);
                if(parser->hasError()) {
                    close();
                    return nullptr;
                }
                len -= nparse;
                if(len == (int)buff_size) {
                    close();
                    return nullptr;
                }
            } while(!parser->isFinished());
            len -= 2;
            
            SYLAR_LOG_INFO(g_logger) << "content_len=" << client_parser.content_len;
            if(client_parser.content_len <= len) {
                body.append(data, client_parser.content_len);
                memmove(data, data + client_parser.content_len
                        , len - client_parser.content_len);
                len -= client_parser.content_len;
            } else {
                body.append(data, len);
                int left = client_parser.content_len - len;
                while(left > 0) {
                    int rt = read(data, left > (int)buff_size ? (int)buff_size : left);
                    if(rt <= 0) {
                        close();
                        return nullptr;
                    }
                    body.append(data, rt);
                    left -= rt;
                }
                len = 0;
            }
        } while(!client_parser.chunks_done);
        parser->getData()->setBody(body);
    } else {
        int64_t length = parser->getContentLength();
        if(length > 0) {
            std::string body;
            body.resize(length);

            int len = 0;
            if(length >= offset) {
                memcpy(&body[0], data, offset);
                len = offset;
            } else {
                memcpy(&body[0], data, length);
                len = length;
            }
            length -= offset;
            if(length > 0) {
                if(readFixSize(&body[len], length) <= 0) {
                    close();
                    return nullptr;
                }
            }
            parser->getData()->setBody(body);
        }
    }
    return parser->getData();
}

/// 发送HTTP响应
int HttpConnection::sendRequest(HttpRequest::ptr rsp) {
    std::stringstream ss;
    ss << *rsp;
    std::string data = ss.str();
    
    return writeFixSize(data.c_str(), data.size());
}

}
}