#include "fd_manager.h"
#include "fd_manager.h"
#include "hook.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

namespace sylar {

FdCtx::FdCtx(int fd)
    :m_isInit(false)
    ,m_isSocket(false)
    ,m_sysNonblock(false)
    ,m_userNonblock(false)
    ,m_isClosed(false)
    ,m_fd(fd)
    ,m_recvTimeout(-1)
    ,m_sendTimeout(-1) {
    init();
}

FdCtx::~FdCtx() {

}    

void FdCtx::setTimeout(int type, uint64_t v) {
    if(type == SO_RCVTIMEO) {   // SO_RCVTIMEO: 接收数据超时时间
        m_recvTimeout = v;
    } else {    // SO_SNDTIMEO: 发送数据超时时间
        m_sendTimeout = v;
    }
}

uint64_t FdCtx::getTimeout(int type) {
    if(type == SO_RCVTIMEO) {
        return m_recvTimeout;
    } else {
        return m_sendTimeout;
    }
}

bool FdCtx::init() {
    if(m_isInit) {
        return true;
    }

    m_recvTimeout = -1;
    m_sendTimeout = -1;

    // 判断fd是否为文件 or Socket
    struct stat fd_stat;
    if(-1 == fstat(m_fd, &fd_stat)) {
        m_isInit = false;
        m_isSocket = false;
    } else {
        m_isInit = true;
        m_isSocket = S_ISSOCK(fd_stat.st_mode);
    } 

    if (m_isSocket) {
        int flags = fcntl_f(m_fd, F_GETFL, 0);  // 获得这个socket的属性

        if(!(flags & O_NONBLOCK)) {    // 如果这个Socket没有被设置为非阻塞的，则由我们来设置
            fcntl_f(m_fd, F_SETFL, flags | O_NONBLOCK);
        }
        m_sysNonblock = true;       // ?? 这个成员变量由啥用呢???
    } else {
         m_sysNonblock = false;
    }

    m_userNonblock = false;     // ??? 有啥用，这里不管怎样都是false
    m_isClosed = false;
    return m_isInit;      
}

FdManager::FdManager() {
    m_datas.resize(64);
}

/// 获取/创建文件句柄类FdCtx 
/// auto_create: 是否自动创建
FdCtx::ptr FdManager::get(int fd, bool auto_create) {
    RWMutexType::ReadLock lock(m_mutex);
    if((int)m_datas.size() <= fd) {
        if(auto_create == false) {  // 找不到，且不需要自动创建
            return nullptr;
        }
    } else {        // 找到了
        if(m_datas[fd] || !auto_create) {   // m_datas[fd]非空 or 不需要自动创建
            return m_datas[fd];     // ? 如果这里是因为auto_create=false进入，可能m_datas[fd]这个智能指针并没有绑定东西怎么办呢??
        }
    }
    lock.unlock();


    RWMutexType::WriteLock lock2(m_mutex);
    FdCtx::ptr ctx(new FdCtx(fd));
    if(fd >= (int)m_datas.size()) {
        m_datas.resize(fd * 1.5);       // 扩容
    }
    m_datas[fd] = ctx;  // 且自动创建、
    return ctx;
}

void FdManager::del(int fd) {
    RWMutexType::WriteLock lock(m_mutex);
    if((int)m_datas.size() <= fd) {
        return;
    }

    m_datas[fd].reset();        // 置空这个智能指针
}       
}