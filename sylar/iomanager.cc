#include "iomanager.h"
#include "macro.h"
#include "log.h"

#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <string.h>

namespace sylar {

static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

IOManager::FdContext::EventContext& IOManager::FdContext::getContext(IOManager::Event event) {
    switch(event) {
        case IOManager::READ:
            return read;
        case IOManager::WRITE:
            return write;
        default:        // 不可能是读和写吗?
            SYLAR_ASSERT2(false, "getContext");
    }
    throw std::invalid_argument("getContext invalid event");
}

/// 重置该fd上的某个事件上下文
void IOManager::FdContext::resetContext(EventContext& ctx) {
    ctx.scheduler = nullptr;
    ctx.fiber.reset();
    ctx.cb = nullptr;
}

void IOManager::FdContext::triggerEvent(IOManager::Event event) {
    // SYLAR_ASSERT(events & event);   // 确保有这个监听事件

    // events = (Event)(events & ~event);  // 删除这个事件

    // EventContext& ctx = getContext(event);
    // if(ctx.cb) {
    //     ctx.scheduler->schedule(&ctx.cb);   // 将其加入调度器中，让某个线程中的协程执行一下这个任务
    // } else {
    //     ctx.scheduler->schedule(&ctx.fiber);
    // }


     SYLAR_ASSERT(events & event);

    events = (Event)(events & ~event);
    EventContext& ctx = getContext(event);
    if(ctx.cb) {
        ctx.scheduler->schedule(&ctx.cb);
    } else {
        ctx.scheduler->schedule(&ctx.fiber);
    }
    ctx.scheduler = nullptr;
    return;
}

IOManager::IOManager(size_t threads, bool use_caller, const std::string& name) 
    : Scheduler(threads, use_caller, name){
    // 创建epoll
    m_epfd = epoll_create(5000);
    SYLAR_ASSERT(m_epfd > 0);

    int rt = pipe(m_tickleFds);
    SYLAR_ASSERT(!rt);

    epoll_event event;
    memset(&event, 0 ,sizeof(event));
    event.events = EPOLLIN | EPOLLET;       // 边缘触发
    event.data.fd = m_tickleFds[0];
    rt = fcntl(m_tickleFds[0], F_SETFL, O_NONBLOCK);    // 设置为非阻塞
    SYLAR_ASSERT(!rt);

    rt = epoll_ctl(m_epfd, EPOLL_CTL_ADD, m_tickleFds[0], &event);  // 加入监听
    SYLAR_ASSERT(!rt);

    contextResize(32);

    start();    // 默认启动Scheduler::start()
    
}
IOManager::~IOManager() {
    stop();

    close(m_epfd);
    close(m_tickleFds[0]);
    close(m_tickleFds[1]);

    // 释放内存
    for(size_t i = 0; i < m_fdContexts.size(); ++i) {
        if(m_fdContexts[i]) {
            delete m_fdContexts[i];
        }
    }
}

void IOManager::contextResize(size_t size) {
    m_fdContexts.resize(size);

    for(size_t i = 0; i < m_fdContexts.size(); ++i) {
        if(!m_fdContexts[i]) {
            // 提前初始化
            m_fdContexts[i] = new FdContext;    
            m_fdContexts[i]->fd = i;
        }
    }
}

int IOManager::addEvent(int fd, Event event, std::function<void()> cb) {
    // SYLAR_LOG_INFO(g_logger) << " addEvent";
    FdContext* fd_ctx = nullptr;
    RWMutexType::ReadLock lock(m_mutex);    // 加锁,可能有多线程同时加入事件
    if((int)m_fdContexts.size() > fd) {
        // fd就是m_fdContexts的下标
        fd_ctx = m_fdContexts[fd];
        lock.unlock();
    } else {
        lock.unlock();
        // 创建
        RWMutexType::WriteLock lock2(m_mutex);
        contextResize(fd * 1.5);
        fd_ctx = m_fdContexts[fd];
    }

    FdContext::MutexType::Lock lock2(fd_ctx->mutex);
    if(fd_ctx->events & event) {   // 事件相同
        // 这个事件可能被其他线程加入过
       SYLAR_LOG_ERROR(g_logger) << "addEvent error" << fd
                                 << "event= " << event
                                 << " fd_ctx.event=" << fd_ctx->events;
        SYLAR_ASSERT(!(fd_ctx->events & event));
    }

    int op = fd_ctx->events ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;    // 判断是修改还是增加
    epoll_event epevent;
    epevent.events = EPOLLET | fd_ctx->events | event;
    epevent.data.ptr = fd_ctx;      // ???保存该事件的fdcContexts

    int rt = epoll_ctl(m_epfd, op, fd, &epevent);
    if (rt) {
        SYLAR_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd <<","
            << op << ", " << fd << ", " << epevent.events<<")"
            << rt << " (" << errno <<") (" <<strerror(errno) << ")";

        return -1;
    }

    ++m_pendingEventCount;      //? 如果这个事件本身就有，那这个数字不就多加了???

    fd_ctx->events = (Event)(fd_ctx->events | event);   // 理解这里的或： 可读可写事件

    FdContext::EventContext& event_ctx = fd_ctx->getContext(event);
    SYLAR_ASSERT(!event_ctx.scheduler   // 确保为空
                && !event_ctx.fiber
                && !event_ctx.cb);
    // ?? 这里不是可以修改的吗? 为什们不允许之前有值呢?

    event_ctx.scheduler = Scheduler::GetThis();     // 设置为当前Scheduler
    if(cb) {
        event_ctx.cb.swap(cb);
    } else {
        // 如果没有指定可调用对象，则将当前协程作为唤醒对象, 发生该事件时程序会切换到当前协程中运行
        event_ctx.fiber = Fiber::GetThis();
        SYLAR_ASSERT(event_ctx.fiber->getState() == Fiber::EXEC);
    }
    // SYLAR_LOG_INFO(g_logger) << " addEvent";
    return 0;
}

bool IOManager::delEvent(int fd, Event event) {
    RWMutexType::ReadLock lock(m_mutex);
    if((int)m_fdContexts.size() <= fd) {
        return false;
    }
    FdContext* fd_ctx = m_fdContexts[fd];   // 获得这个fd的上下文
    lock.unlock();

    FdContext::MutexType::Lock lock2(fd_ctx->mutex);
    if(fd_ctx->events & event) {    // fd中没有这个事件
        return false;   // 则不用删除
    }

    Event new_events = (Event)(fd_ctx->events & ~event);
    int op = new_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;        // 判断是修改这个fd,还是直接删除fd

    // 重新设置这个fd上的事件
    epoll_event epevent;
    epevent.events = EPOLLET | new_events;
    epevent.data.ptr = fd_ctx;

    // 重新设置这个fd的监听情况
    int rt = epoll_ctl(m_epfd, op, fd, &epevent);
    if (rt) {
        SYLAR_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd <<","
            << op << ", " << fd << ", " << epevent.events<<")"
            << rt << " (" << errno <<") (" <<strerror(errno) << ")";

        return false;
    }

    --m_pendingEventCount;

    fd_ctx->events = new_events;
    FdContext::EventContext& event_ctx = fd_ctx->getContext(event);
    fd_ctx->resetContext(event_ctx);
    return true;
}
bool IOManager::cancelEvent(int fd, Event event) {
    // SYLAR_LOG_ERROR(g_logger) << " cancelEvent begin";
    RWMutexType::ReadLock lock(m_mutex);
    if((int)m_fdContexts.size() <= fd) {
        return false;
    }
    FdContext* fd_ctx = m_fdContexts[fd];   // 获得这个fd的上下文
    lock.unlock();

    FdContext::MutexType::Lock lock2(fd_ctx->mutex);
    if(!(fd_ctx->events & event)) {    // fd中没有这个事件
        return false;   // 则不用删除
    }

    Event new_events = (Event)(fd_ctx->events & ~event);
    int op = new_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;        // 判断是修改这个fd,还是直接删除fd

    // 重新设置这个fd上的事件
    epoll_event epevent;
    epevent.events = EPOLLET | new_events;
    epevent.data.ptr = fd_ctx;

    // 重新设置这个fd的监听情况
    int rt = epoll_ctl(m_epfd, op, fd, &epevent);
    if (rt) {
        SYLAR_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd <<","
            << op << ", " << fd << ", " << epevent.events<<")"
            << rt << " (" << errno <<") (" <<strerror(errno) << ")";

        return false;
    }
    // SYLAR_LOG_ERROR(g_logger) << " cancelEvent end ";
    fd_ctx->triggerEvent(event);
    --m_pendingEventCount;
    return true;
}

bool IOManager::cancelAll(int fd){
    
    RWMutexType::ReadLock lock(m_mutex);
    if((int)m_fdContexts.size() <= fd) {
        return false;
    }
    FdContext* fd_ctx = m_fdContexts[fd];
    lock.unlock();

    FdContext::MutexType::Lock lock2(fd_ctx->mutex);
    if(!fd_ctx->events) {       // 如果这个fd本身就没有事件
        return false;
    }

    int op = EPOLL_CTL_DEL;     // 删除
    epoll_event epevent;
    epevent.events = 0;
    epevent.data.ptr = fd_ctx;

    int rt = epoll_ctl(m_epfd, op, fd, &epevent);
    if(rt) {
        SYLAR_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", "
            << op << ", " << fd << ", " << epevent.events << "):"
            << rt << " (" << errno << ") (" << strerror(errno) << ")";
        return false;
    }

    if(fd_ctx->events & READ) {
        fd_ctx->triggerEvent(READ);
        // FdContext::EventContext& event_ctx = fd_ctx->getContext(READ);
        // event_ctx->triggerEvent(event);     // 如果该事件被注册过回调，那就触发一次回调事件
        --m_pendingEventCount;
    }
    if(fd_ctx->events & WRITE) {
        fd_ctx->triggerEvent(WRITE);
        // FdContext::EventContext& event_ctx = fd_ctx->getContext(WRITE);
        // event_ctx->triggerEvent(event);     // 如果该事件被注册过回调，那就触发一次回调事件
        --m_pendingEventCount;
    }

    SYLAR_ASSERT(fd_ctx->events == 0);
    return true;
}

IOManager* IOManager::GetThis() {
    // 返回当前线程使用的IOManager
    return dynamic_cast<IOManager*>(Scheduler::GetThis());
}

/// 有事件发生，则可以唤醒空闲的线程
void IOManager::tickle() {
    if(!hasIdleThreads()) {     // 必须有闲置的线程才有意义
        return;
    }
    // SYLAR_LOG_INFO(g_logger) << " IOManager::tickle ";
    int rt = write(m_tickleFds[1], "T", 1);     // 往管道中写一个数据，通知有事件加入
    SYLAR_ASSERT(rt == 1);
}


bool IOManager::stopping(uint64_t& timeout) {
    timeout = getNextTimer();
    return timeout == ~0ull
        && m_pendingEventCount == 0
        && Scheduler::stopping();

}

bool IOManager::stopping() {
    uint64_t timeout = 0;
    return stopping(timeout);
    // return m_pendingEventCount == 0      // 所有的事件都完成了
    //     && hasTimer()           // 并且没有定时器事件
    //     && Scheduler::stopping();       // Scheduler的stop条件满足
}

/// 核心
/// 当线程什么都不干的时候，就会陷入idle。基类的idle就是在主协程和idle中不断的切换，是不合理的
void IOManager::idle() {

    const uint64_t MAX_EVNETS = 64;
    epoll_event* events = new epoll_event[MAX_EVNETS]();    // 利用new分配数组
    std::shared_ptr<epoll_event> shared_events(events, [](epoll_event* ptr){
        delete[] ptr;       // 提供析构的方法
    });

    while(true) {
        uint64_t next_timeout = 0;
        if (stopping(next_timeout)) {   // 调度器结束了
            next_timeout = getNextTimer();
            SYLAR_LOG_INFO(g_logger) << "name = " << getName() << "idle stopping exit";
            break;
        }

        
        int rt = 0;
        do {
            static const int MAX_TIMEOUT = 1000;    // 毫秒级的超时时间
            if (next_timeout  != ~0ull) {
                // 这里可能next_timeout=0???????
                next_timeout = (int)next_timeout > MAX_TIMEOUT ? MAX_TIMEOUT : next_timeout;
            } else {
                next_timeout = MAX_TIMEOUT;
            }
            //  SYLAR_LOG_DEBUG(g_logger) << "next_timeout = " << next_timeout;
            // epool_wait的超时时间会参考最近的定时器任务
            rt = epoll_wait(m_epfd, events, MAX_EVNETS, (int)next_timeout);   // 如果没有事件，则陷入epoll_wait
            if (rt < 0 && errno == EINTR) {

            } else {
                // 超时
                break;
            }
        } while(true);

        // 检查满足条件的定时器任务
        std::vector<std::function<void()> > cbs;
        listExpiredCb(cbs);
        if (!cbs.empty()) {
            // SYLAR_LOG_DEBUG(g_logger) << "on timer cbs.size=" << cbs.size();
            schedule(cbs.begin(), cbs.end());    // 把定时任务加入调度
            // for(auto& i : cbs)
            //     schedule(i);
            cbs.clear();
        }

        // 此时rt返回所有就绪的事件的数量
        // SYLAR_LOG_INFO(g_logger) << "epoll wait events=" << rt << " pendingevnet = " << m_pendingEventCount;
        for (int i = 0; i < rt; i++) {
            epoll_event& event = events[i];
            if(event.data.fd == m_tickleFds[0]) {   // 说明外部发消息来唤醒
            //  SYLAR_LOG_INFO(g_logger) << "??epoll wait events=" << rt << " pendingevnet = " << m_pendingEventCount;
                uint8_t dummy[256];
                while(read(m_tickleFds[0], dummy, sizeof(dummy)) > 0);     // 收消息，while确保读干净
                // ?? 还是不明白用信号量和管道通信的区别?
                continue;
            }

            FdContext* fd_ctx = (FdContext*)event.data.ptr;     // 取出fd事件上下文
            FdContext::MutexType::Lock lock(fd_ctx->mutex);
            if (event.events & (EPOLLERR | EPOLLHUP)) {
                event.events |= (EPOLLIN | EPOLLOUT) & fd_ctx->events;  // ???
            }

            int real_events = NONE;
            if(event.events & EPOLLIN) {
                real_events |= READ;
            }
            if(event.events & EPOLLOUT) {
                real_events |= WRITE;
            }

            if((fd_ctx->events & real_events) == NONE) {    // 怎么会出现None?
                continue;
            }

            // 剩余事件
            int left_events = (fd_ctx->events & ~real_events);
            int op = left_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
            event.events = EPOLLET | left_events;  // 复用这个event,该fd上可能还有其他事件

            int rt2 = epoll_ctl(m_epfd, op, fd_ctx->fd, &event);
            if(rt2) {
                SYLAR_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", "
                    << op << ", " << fd_ctx->fd << ", " << event.events << "):"
                    << rt2 << " (" << errno << ") (" << strerror(errno) << ")";
                continue;
            }

            // 触发事件
            if(real_events & READ) {
                fd_ctx->triggerEvent(READ);
                --m_pendingEventCount;
            }
            if(real_events & WRITE) {
                fd_ctx->triggerEvent(WRITE);
                --m_pendingEventCount;
            }
        }

            // 触发的事件都通过schduler()加入了任务队列，则可以让出控制权
        Fiber::ptr cur = Fiber::GetThis();
        auto raw_ptr = cur.get();
        cur.reset();

        raw_ptr->swapOut();  // 切换的线程的main协程/调度协程，执行run
    }

}

void IOManager::onTimerInsertedAtFront() {
    SYLAR_LOG_INFO(g_logger) << "?";
    // 一旦调用该函数，就应该重新设定epoll_wait的等待时间
    tickle();   // 唤醒epoll_wite的线程
}


}