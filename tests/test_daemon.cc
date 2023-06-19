#include "../sylar/daemon.h"
#include "../sylar/iomanager.h"
#include "../sylar/log.h"

static sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

sylar::Timer::ptr timer;
int server_main(int argc, char** argv) {
    SYLAR_LOG_INFO(g_logger) << sylar::ProcessInfoMgr::GetInstance()->toString();

    sylar::IOManager iom(1);      // 启动IOManager
    timer = iom.addTimer(1000, [](){
            SYLAR_LOG_INFO(g_logger) << "onTimer";
            static int count = 0;
            if(++count > 10) {
                // exit(1);    // 异常退出， 会继续fork()进程
                exit(0);    // 正常退出
            }
    }, true);

    return 0;
}

int main(int argc, char** argv) {
    return sylar::start_daemon(argc, argv, server_main, argc != 1); // 没有参数就执行后台
}