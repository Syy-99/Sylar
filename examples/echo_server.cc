#include "../sylar/tcp_server.h"
#include "../sylar/log.h"
#include "../sylar/iomanager.h"
#include "../sylar/bytearray.h"
#include "../sylar/address.h"

static sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

// 使用示例: 基于我们的框架，实现一个回声服务器
class EchoServer : public sylar::TcpServer {
public:
    EchoServer(int type);
    void handleClient(sylar::Socket::ptr client);

private:
    int m_type = 0;     // 输出形式：文本形式 or 二进制形式
};

EchoServer::EchoServer(int type)
    :m_type(type) {
}

// 主要实现这个handlerClent方法，来处理新的TCP连接
void EchoServer::handleClient(sylar::Socket::ptr client) {
    SYLAR_LOG_INFO(g_logger) << "handleClient " << *client;
    sylar::ByteArray::ptr ba(new sylar::ByteArray);
    while(true) {
        ba->clear();

        std::vector<iovec> iovs;
        ba->getWriteBuffers(iovs, 1024);        // 给ByteArray分配空间， 并且会创建对应的iovec结构
        // ??? 这里感觉ByteArray除了使用链表来逻辑化连续空间，好像没有用到它的压缩特性呀????
    
        // 开始接受数据
        int rt = client->recv(&iovs[0], iovs.size());   // 异步recv
        if(rt == 0) {   // 连接断开
            SYLAR_LOG_INFO(g_logger) << "client close: " << *client;
            break;
        } else if(rt < 0) { // 出错
            SYLAR_LOG_INFO(g_logger) << "client error rt=" << rt
                << " errno=" << errno << " errstr=" << strerror(errno);
            break;
        }

        // 读到数据了，保存在ByteArray中
        // 因为getWriteBuffers并不会修改position or size, 所以我们先需要通过setPosition来设置实际的size大小
        // 这样后续才知道从起始位置可以读多少内容
        ba->setPosition(ba->getPosition() + rt);     
        // SYLAR_LOG_INFO << "???" << m_type;
        ba->setPosition(0);      // 设置读取的起始位置
        if(m_type == 1) {      //要求输出文本形式，text 
            std::cout << ba->toString() << std::endl;
        } else {
            std::cout << ba->toHexString() << std::endl;
        }
        std::cout.flush(); // 刷新流
    }
}

int type = 1;

void run() {
    SYLAR_LOG_INFO(g_logger) << "server type=" << type;
    EchoServer::ptr es(new EchoServer(type));   // 创建回声服务器
    auto addr = sylar::Address::LookupAny("0.0.0.0:8020");  // 给服务器创建监听套接字
    while(!es->bind(addr)) {
        sleep(2);
    }
    es->start();    // 启动服务器
}

int main(int argc, char** argv){
    if(argc < 2) {
        SYLAR_LOG_INFO(g_logger) << "used as[" << argv[0] << " -t] or [" << argv[0] << " -b]";
        return 0;
    }

    if(!strcmp(argv[1], "-b")) {
        type = 2;
    }

    sylar::IOManager iom(2);
    iom.schedule(run);     
    return 0;
}