#include "daemon.h"
#include "log.h"
#include "config.h"
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace sylar{

static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

// 服务重启时间间隔
static sylar::ConfigVar<uint32_t>::ptr g_daemon_restart_interval
    = sylar::Config::Lookup("daemon.restart_interval", (uint32_t)5, "daemon restart interval");

std::string ProcessInfo::toString() const {
    std::stringstream ss;
    ss << "[ProcessInfo parent_id=" << parent_id
       << " main_id=" << main_id
       << " parent_start_time=" << sylar::Time2Str(parent_start_time)
       << " main_start_time=" << sylar::Time2Str(main_start_time)
       << " restart_count=" << restart_count << "]";
    return ss.str();
}


static int real_start(int argc, char** argv,
                     std::function<int(int argc, char** argv)> main_cb) {
    // ?? 这里还需要在负值一次吗???
    ProcessInfoMgr::GetInstance()->main_id = getpid();  // 获得主线程id
    ProcessInfoMgr::GetInstance()->main_start_time = time(0);   // 设置主线程的启动时间戳
    return main_cb(argc, argv);
}

static int real_daemon(int argc, char** argv,
                     std::function<int(int argc, char** argv)> main_cb) {
    daemon(1, 0);   // 执行守护进程函数，(工作路径，是否关闭标准输入输出流)
    ProcessInfoMgr::GetInstance()->parent_id = getpid();
    ProcessInfoMgr::GetInstance()->parent_start_time = time(0);  

    while(true) {  // 死循环
        pid_t pid = fork();     // 创建子进程
        if (pid == 0) {
            // 子进程返回
            ProcessInfoMgr::GetInstance()->main_id = getpid();
            ProcessInfoMgr::GetInstance()->main_start_time  = time(0);
            SYLAR_LOG_INFO(g_logger) << "process start pid=" << getpid();
            return real_start(argc, argv, main_cb);    // 启动程序
        } else if (pid < 0){
            SYLAR_LOG_ERROR(g_logger) << "fork fail return=" << pid
                << " errno=" << errno << " errstr=" << strerror(errno);
            return -1;
        } else {
            // 父进程返回
             int status = 0;
            waitpid(pid, &status, 0);       // 阻塞等待子进程结束
            if(status) {
                if(status == 9) {
                    SYLAR_LOG_INFO(g_logger) << "killed";
                    break;
                } else {
                    SYLAR_LOG_ERROR(g_logger) << "child crash pid=" << pid
                        << " status=" << status;
                }
            } else {
                SYLAR_LOG_INFO(g_logger) << "child finished pid=" << pid;
                break;
            }
            ProcessInfoMgr::GetInstance()->restart_count += 1;
            sleep(g_daemon_restart_interval->getValue());   // 睡眠后再启动
        }
    }                      
    return 0;
}

int start_daemon(int argc, char** argv
                 , std::function<int(int argc, char** argv)> main_cb
                 , bool is_daemon) {
    if(!is_daemon) {    // 如果不是守护进程，则直接启动
        return real_start(argc, argv, main_cb);
    }

    return real_daemon(argc, argv, main_cb);
}

}