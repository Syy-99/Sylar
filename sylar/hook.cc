#include "hook.h"
#include "fiber.h"
#include "iomanager.h"

#include <dlfcn.h>
namespace sylar {

static thread_local bool t_hook_enable = false;     // 线程局部变量，判断该线程是否执行Hook

#define HOOK_FUN(XX) \
    XX(sleep) \
    XX(usleep) \
/// 从库中取出最初的系统调用
void hook_init() {
    static bool is_inited = false;
    if(is_inited) {
        return;
    }
#define XX(name) name ## _f = (name ## _fun)dlsym(RTLD_NEXT, #name);    //  name ## _f -> <name>_f
    HOOK_FUN(XX); 
#undef XX    
}

struct _HookIniter {
    _HookIniter() {
        hook_init();       // 绑定hook
        // s_connect_timeout = g_tcp_connect_timeout->getValue();

        // g_tcp_connect_timeout->addListener([](const int& old_value, const int& new_value){
        //         SYLAR_LOG_INFO(g_logger) << "tcp connect timeout changed from "
        //                                  << old_value << " to " << new_value;
        //         s_connect_timeout = new_value;
        // });
    }
};
static _HookIniter s_hook_initer;



bool is_hook_enable() {
    return t_hook_enable;
}

void set_hook_enable(bool flag) {
    t_hook_enable = flag;
}



extern "C" {

#define XX(name) name ## _fun name ## _f = nullptr;
    HOOK_FUN(XX);       // 声明变量
#undef XX   

/// 采用和系统调用完全相同的函数声明
unsigned int sleep(unsigned int seconds) {
    if(!sylar::t_hook_enable) {
        // 如果我们没有启动Hook, 则直接调用系统调用即可
        return sleep_f(seconds);
    }

    sylar::Fiber::ptr fiber = sylar::Fiber::GetThis();  // 取出当前协程
    sylar::IOManager* iom = sylar::IOManager::GetThis();  // 取出当前IOmanager
    // iom->addTimer(seconds * 1000, std::bind(&sylar::IOManager::schedule, iom, fiber));       // 添加定时器任务，睡眠结束就将该协程重新调度
    iom->addTimer(seconds * 1000, [iom, fiber] () {
        iom->schedule(fiber);
    });
    sylar::Fiber::YieldToHold();  // 当前协程放弃CPU，则可以运行其他协程

    return 0;
}

int usleep(useconds_t usec) {
    if(!sylar::t_hook_enable) {
        // 如果我们没有启动Hook, 则直接调用系统调用即可
        return usleep_f(usec);
    }

    sylar::Fiber::ptr fiber = sylar::Fiber::GetThis();  // 取出当前协程
    sylar::IOManager* iom = sylar::IOManager::GetThis();  // 取出当前IOmanager
    // iom->addTimer(seconds / 1000, std::bind(&sylar::IOManager::schedule, iom, fiber));       // 添加定时器任务，睡眠结束就将该协程重新调度
    iom->addTimer(usec / 1000, [iom, fiber] () {
        iom->schedule(fiber);
    });
    sylar::Fiber::YieldToHold();  // 当前协程放弃CPU，则可以运行其他协程

    return 0;
}

}   // extern "c"

} // sylar
