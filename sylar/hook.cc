#include "hook.h"
#include "fiber.h"
#include "iomanager.h"
#include "fd_manager.h"
#include <dlfcn.h>
#include "log.h"

sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

namespace sylar {

static thread_local bool t_hook_enable = false;     // 线程局部变量，判断该线程是否执行Hook

#define HOOK_FUN(XX) \
    XX(sleep) \
    XX(usleep) \
    XX(nanosleep) \
    XX(socket) \
    XX(connect) \
    XX(accept) \
    XX(read) \
    XX(readv) \
    XX(recv) \
    XX(recvfrom) \
    XX(recvmsg) \
    XX(write) \
    XX(writev) \
    XX(send) \
    XX(sendto) \
    XX(sendmsg) \
    XX(close) \
    XX(fcntl) \
    XX(ioctl) \
    XX(getsockopt) \
    XX(setsockopt)
    
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

/// 存放条件的结构体，为了条件定时器
/// ??? 为什么要用结构体，直接用int类型变量不可以吗??
struct timer_info {
    int cancelled = 0;
};


/// 读写的Socket函数封装
// fun: 原始的函数；hook_fun_name: 函数名； event: 事件类型；timeout_so： time_out的类型
// Args: fun函数需要的参数（可变）
template<typename OriginFun, typename... Args>
static ssize_t do_io(int fd, OriginFun fun, const char* hook_fun_name,
                     uint32_t event, int timeout_so, Args&&... args) {
    if (!sylar::t_hook_enable) {    // 判断是否hook
        return fun(fd, std::forward<Args>(args)...);
    }

    sylar::FdCtx::ptr ctx = sylar::FdMgr::GetInstance()->get(fd);
    if(!ctx) {      // 如果不存在，就根本不是socket
        return fun(fd, std::forward<Args>(args)...);
    }

    if(ctx->isClose()) {    // 该文件关闭，则有问题
        errno = EBADF;     
        return -1;
    }

    if(!ctx->isSocket() || ctx->getUserNonblock()) {
        // 如果不是Socket, 或者是Socket但是用户已经设置为非阻塞的，则我们也不需要处理!!! why??
        return fun(fd, std::forward<Args>(args)...);
    }

    // 是Socket，并且用户没有设置为非阻塞

    uint64_t to = ctx->getTimeout(timeout_so);    // 获得超时时间

    std::shared_ptr<timer_info> tinfo(new timer_info);  // 创建条件

retry:
    ssize_t n = fun(fd, std::forward<Args>(args)...); // 执行原始方法

    // 开始异步处理
    while(n == -1 && errno == EINTR) {  // 说明调用因为信号的原因中断
        n = fun(fd, std::forward<Args>(args)...);
    }

    if (n == -1 && errno == EAGAIN) {   // 非阻塞但是条件不满足，就会出现EAGAIN错误
        // 开始异步管理
        sylar::IOManager* iom = sylar::IOManager::GetThis();
        sylar::Timer::ptr timer;
        std::weak_ptr<timer_info> winfo(tinfo);    // 条件

        if (to != (uint64_t)-1) {   // 说明设置了超时时间
            timer = iom->addConditionTimer(to, [winfo, fd, iom, event]() {
                auto t = winfo.lock();  // 检查条件是否还存在
                if(!t || t->cancelled) {    // 如果不存在 或 被取消 cancelled ??? 在哪里???
                    return;
                }
                t->cancelled = ETIMEDOUT;       // 设置超时时间
                iom->cancelEvent(fd, (sylar::IOManager::Event)(event));  //  取消fd上的事件
            }, winfo);
        }

        int rt = iom->addEvent(fd, (sylar::IOManager::Event)(event));   // 添加事件,但是不需要指定cb
        if (rt) {       // 添加事件操作出错
            SYLAR_LOG_ERROR(g_logger) << hook_fun_name << " addEvent("
                << fd << ", " << event << ")";
            
            if(timer) {     // 还需要取消定时器，会在定时器容器类型中删除该定时器对象
                timer->cancel();
            }
            return -1;
        } else {    // 事件添加成功
            sylar::Fiber::YieldToHold();    // 放弃CPU, 可以执行其他任务

            if(timer) { // 如果提前唤醒，则取消timer
                timer->cancel();
            }
            if(tinfo->cancelled) {      // 说明等待超时
                errno = tinfo->cancelled;
                return -1;
            }

            goto retry;     // 说明是因为超时而返回，所以还需要继续等
        }
    }

    return n;   // 如果读到数据，则可以直接返回
    // 这个时候虽然对应的timer没有被手动释放，但是winfo是空，因此即使该timer到达时间触发执行，也不会做任何操作
    // 反而，如果到达超时时间还没有读到数据，则会执行其中的函数取消事件                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            
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
    // iom->addTimer(seconds * 1000, [iom, fiber] () {
    //     iom->schedule(fiber);
    // });
    iom->addTimer(seconds * 1000, 
            std::bind(
            (void(sylar::Scheduler::*)      // 进行强制类型转换为基类
            (sylar::Fiber::ptr, int thread))&sylar::IOManager::schedule  // 模版方法要声明类型
            ,iom, fiber, -1)
            );
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
    // 添加定时器任务，睡眠结束就将该协程重新调度
    // iom->addTimer(usec / 1000, [iom, fiber] () {
    //     iom->schedule(fiber);
    // });
    iom->addTimer(usec / 1000, std::bind((void(sylar::Scheduler::*)
        (sylar::Fiber::ptr, int thread))&sylar::IOManager::schedule
        ,iom, fiber, -1));
    sylar::Fiber::YieldToHold();  // 当前协程放弃CPU，则可以运行其他协程

    return 0;
}

int nanosleep(const struct timespec *req, struct timespec *rem) {
    if(!sylar::t_hook_enable) {
        return nanosleep_f(req, rem);
    }

    int timeout_ms = req->tv_sec * 1000 + req->tv_nsec / 1000 /1000;    // 可能有精度问题
    sylar::Fiber::ptr fiber = sylar::Fiber::GetThis();
    sylar::IOManager* iom = sylar::IOManager::GetThis();
    // iom->addTimer(timeout_ms, [iom, fiber] () {
    //     iom->schedule(fiber);
    // });
    iom->addTimer(timeout_ms, std::bind((void(sylar::Scheduler::*)
        (sylar::Fiber::ptr, int thread))&sylar::IOManager::schedule
        ,iom, fiber, -1));
    sylar::Fiber::YieldToHold();
    return 0;
}


int socket(int domain, int type, int protocol) {
    if(!sylar::t_hook_enable) {
        return socket_f(domain, type, protocol);
    }
    int fd = socket_f(domain, type, protocol);
    if(fd == -1) {
        return fd;
    }
    sylar::FdMgr::GetInstance()->get(fd, true);     // 所有通过Socket()创建的fdd都会保存起来，方便我们确定fd的类型
    return fd;
}


int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    // return connect_with_timeout(sockfd, addr, addrlen, sylar::s_connect_timeout);
    return 0;
}


int accept(int s, struct sockaddr *addr, socklen_t *addrlen) {
    // accpet实际上相当于从监听套接字中执行读操作
    int fd = do_io(s, accept_f, "accept", sylar::IOManager::READ, SO_RCVTIMEO, addr, addrlen);
    if(fd >= 0) {       // 读到的，则会返回一个socket套接字
        sylar::FdMgr::GetInstance()->get(fd, true);     // 加入记录中
    }
    return fd;
}


ssize_t read(int fd, void *buf, size_t count) {
    return do_io(fd, read_f, "read", sylar::IOManager::READ, SO_RCVTIMEO, buf, count);
}

ssize_t readv(int fd, const struct iovec *iov, int iovcnt) {
    return do_io(fd, readv_f, "readv", sylar::IOManager::READ, SO_RCVTIMEO, iov, iovcnt);
}

ssize_t recv(int sockfd, void *buf, size_t len, int flags) {
    return do_io(sockfd, recv_f, "recv", sylar::IOManager::READ, SO_RCVTIMEO, buf, len, flags);
}

ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen) {
    return do_io(sockfd, recvfrom_f, "recvfrom", sylar::IOManager::READ, SO_RCVTIMEO, buf, len, flags, src_addr, addrlen);
}

ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags) {
    return do_io(sockfd, recvmsg_f, "recvmsg", sylar::IOManager::READ, SO_RCVTIMEO, msg, flags);
}

ssize_t write(int fd, const void *buf, size_t count) {
    return do_io(fd, write_f, "write", sylar::IOManager::WRITE, SO_SNDTIMEO, buf, count);
}

ssize_t writev(int fd, const struct iovec *iov, int iovcnt) {
    return do_io(fd, writev_f, "writev", sylar::IOManager::WRITE, SO_SNDTIMEO, iov, iovcnt);
}

ssize_t send(int s, const void *msg, size_t len, int flags) {
    return do_io(s, send_f, "send", sylar::IOManager::WRITE, SO_SNDTIMEO, msg, len, flags);
}

ssize_t sendto(int s, const void *msg, size_t len, int flags, const struct sockaddr *to, socklen_t tolen) {
    return do_io(s, sendto_f, "sendto", sylar::IOManager::WRITE, SO_SNDTIMEO, msg, len, flags, to, tolen);
}

ssize_t sendmsg(int s, const struct msghdr *msg, int flags) {
    return do_io(s, sendmsg_f, "sendmsg", sylar::IOManager::WRITE, SO_SNDTIMEO, msg, flags);
}


int close(int fd) {
    if(!sylar::t_hook_enable) {
        return close_f(fd);
    }

    sylar::FdCtx::ptr ctx = sylar::FdMgr::GetInstance()->get(fd);
    if(ctx) {       // 如果是socket套接字
        auto iom = sylar::IOManager::GetThis();
        if(iom) {
            iom->cancelAll(fd);     // 取消这个fd上的所有事件
        }
        sylar::FdMgr::GetInstance()->del(fd);   // 删除这个fd记录
    }
    return close_f(fd); // 取消完后，再关闭

}

// hook fcntl有什么意义? 又不能做异步的?
int fcntl(int fd, int cmd, ... /* arg */ )  {
    // 设置non-blcok
}

int ioctl(int d, unsigned long int request, ...) {

}

int getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen) {

}

int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen) {

}


}   // extern "c"

} // sylar
