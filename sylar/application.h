#ifndef __SYLAR_APPLICATION_H__
#define __SYLAR_APPLICATION_H__


#include "http/http_server.h"
namespace sylar {

class Application {
public:
    Application();

    static Application* GetInstance() {
        return s_instance;
    }

    bool init(int argc, char** argv);
    bool run();
private:
    int main(int argc, char** argv);
    int run_fiber();    // 线程运行的主协程
private:
    int m_argc = 0;
    char** m_argv = nullptr;
    static Application* s_instance;

    std::vector<sylar::http::HttpServer::ptr> m_httpservers;    // 记录启动的HttpServer
};
}


#endif