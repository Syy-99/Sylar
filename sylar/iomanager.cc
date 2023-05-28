#include "iomanager.h"
#include "macro.h"
#include "log.h"

#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>

namespace sylar {

static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

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
    if(fb_ctx->events & events) {
       // SYLAR_LOG_ERROR(g_logger) << "state "
    }

}
bool IOManager::delEvent(int fd, Event event);
bool IOManager::cancelEvent(int fd, Event event);

}