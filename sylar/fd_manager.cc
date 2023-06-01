#include "fd_manager.h"

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

}

uint64_t FdCtx::getTimeout(int type) {

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

}

/// auto_create: 是否自动创建
FdCtx::ptr FdManager::get(int fd, bool auto_create = false) {

}

void FdManager::del(int fd) {

}       
}