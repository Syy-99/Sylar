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
    SYLAR_ASSERT(events & event);   // 确保有这个监听事件

    events = (Event)(events & ~event);  // 删除这个事件

    EventContext& ctx = getContext(event);
    if(ctx.cb) {
        ctx.scheduler->schedule(&ctx.cb);   // 将其加入调度器中，让某个线程中的协程执行一下这个任务
    } else {
        ctx.scheduler->schedule(&ctx.fiber);
    }

    ctx.scheduler = nullptr;
    return;
}

IOManager::IOManager(size_t threads = 1, bool use_caller = true, const std::string& name = "") 
    : Scheduler(threads, use_caller, name){
    // 创建epoll
    m_epfd = epoll_create(5000);
    SYLAR_ASSERT(m_epfd > 0);

    int rt = pipe(m_tickleFds);
    SYLAR_ASSERT(!rt);

    epoll_event event;
    memset(&event, 0 ,sizeof(event))
    event.events = EPOLLIN | EPOLLET;
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

void contextResize(size_t size) {
    m_fdContexts.resize(size);

    for(size_t i = 0; i < m_fdContexts.size(); ++i) {
        if(!m_fdContexts[i]) {
            // 提前初始化
            m_fdContexts[i] = new FdContext;    
            m_fdContexts[i]->fd = i;
        }
    }
}

int IOManageradd::addEvent(int fd, Event event, std::function<void()> cb = nullptr) {
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

    FdContext::MutexType::Lock lock2(fb_ctx->mutex);
    if(fb_ctx->events & events) {   // 事件相同
        // 这个事件可能被其他线程加入过
       SYLAR_LOG_ERROR(g_logger) << "addEvent error" << fd
                                 << "event= " << event;
                                 << " fd_ctx.event=" << fb_ctx->events;
        SYLAR_ASSERT(!(fb_ctx->events & event));
    }

    int op = fd_ctx->events ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;    // 判断是修改还是增加
    epoll_event epevent;
    epevent.events = EPOLLET | fd_ctx->events | event;
    epevent.data.ptr = fd_ctx;      // ???保存该事件的fdcContexts

    int rt = epoll_ctl(m_epfd, op, fd, &epevent);
    if (rt) {
        SYLAR_LOG_ERROR << "epoll_ctl(" << m_epfd <<","
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
        event_ctx.fiber = Fiber::GetThis(); // 这里的逻辑是什么？
        SYLAR_ASSERT2(event_ctx.fiber->getState() == Fiber::EXEC);
    }

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
        SYLAR_LOG_ERROR << "epoll_ctl(" << m_epfd <<","
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
        SYLAR_LOG_ERROR << "epoll_ctl(" << m_epfd <<","
            << op << ", " << fd << ", " << epevent.events<<")"
            << rt << " (" << errno <<") (" <<strerror(errno) << ")";

        return false;
    }

    fd_ctx->triggerEvent(event)
    // FdContext::EventContext& event_ctx = fd_ctx->getContext(event);
    // event_ctx->triggerEvent(event);     // 如果该事件被注册过回调，那就触发一次回调事件
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
            << (EpollCtlOp)op << ", " << fd << ", " << (EPOLL_EVENTS)epevent.events << "):"
            << rt << " (" << errno << ") (" << strerror(errno) << ")";
        return false;
    }

    if(fd_ctx->events & READ) {
        fd_ctx->triggerEvent(READ)
        // FdContext::EventContext& event_ctx = fd_ctx->getContext(READ);
        // event_ctx->triggerEvent(event);     // 如果该事件被注册过回调，那就触发一次回调事件
        --m_pendingEventCount;
    }
    if(fd_ctx->events & WRITE) {
        fd_ctx->triggerEvent(WRITE)
        // FdContext::EventContext& event_ctx = fd_ctx->getContext(WRITE);
        // event_ctx->triggerEvent(event);     // 如果该事件被注册过回调，那就触发一次回调事件
        --m_pendingEventCount;
    }

    SYLAR_ASSERT(fd_ctx->events == 0);
    return true;
}

IOManager* GetThis() {
    // 返回当前线程使用的IOManager
    return dynamic_cast<IOManager*>(Scheduler::GetThis());
}

}