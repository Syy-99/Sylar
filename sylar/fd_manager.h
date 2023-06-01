/**
 * @file fd_manager.h
 * @brief 文件句柄管理类
 */
#ifndef __FD_MANAGER_H__
#define __FD_MANAGER_H__

#include <memory>
#include "mutex.h"

namespace sylar {

/**
 * @brief 文件句柄上下文类
 * @details 管理文件句柄类型(是否socket)
 *          是否阻塞,是否关闭,读/写超时时间
 */
class FdCtx : public std::enable_shared_from_this<FdCtx> {
public:
    typedef std::shared_ptr<FdCtx> ptr;

    FdCtx(int fd);
    ~FdCtx();

    bool isInit() const { return m_isInit; }
    bool isSocket() const { return m_isSocket; }
    bool isClose() const { return m_isClosed; }
    bool close();

    void setUserNonblock(bool v) { m_userNonblock = v;}
    bool getUserNonblock() const { return m_userNonblock;}

    // ???啥叫系统非阻塞??? 为什么要区分两种??
    void setSysNonblock(bool v) { m_sysNonblock = v;}
    bool getSysNonblock() const { return m_sysNonblock;}

    /**
     * @brief 设置超时时间
     * @param[in] type 类型SO_RCVTIMEO(读超时), SO_SNDTIMEO(写超时)
     * @param[in] v 时间毫秒
     */
    void setTimeout(int type, uint64_t v);
    uint64_t getTimeout(int type);
 private:
    /**
     * @brief 初始化
     */
    bool init();

private:
    /// 是否初始化
    bool m_isInit;
    bool m_isSocket;
    /// 是否hook非阻塞
    bool m_sysNonblock;
    /// 是否用户主动设置非阻塞
    bool m_userNonblock;
    /// 是否关闭
    bool m_isClosed;
    /// 文件句柄
    int m_fd;
    /// 读超时时间毫秒
    uint64_t m_recvTimeout;
    /// 写超时时间毫秒
    uint64_t m_sendTimeout;    
};

/**
 * @brief 文件句柄管理类
 */
class FdManager {
public:
    typedef RWMutex RWMutexType;   

    FdManager();
    
    /// auto_create: 是否自动创建
    FdCtx::ptr get(int fd, bool auto_create = false);
    void del(int fd);   

private:
    /// 读写锁
    RWMutexType m_mutex;
    /// 文件句柄集合
    std::vector<FdCtx::ptr> m_datas;
};

}

#endif