/**
 * @file iomanager.h
 * @brief 基于Epoll的IO协程调度器
 */
#ifndef __SYLAR_IOMANAGER_H__
#define __SYLAR_IOMANAGER_H__

#include "scheduler.h"

namespace sylar {

class IOManager : public Scheduler {
public:
    typedef std::shared_ptr<IOManager> ptr;
    typedef RWMutex RWMutexType;    // 使用读写锁!

    /// IO事件
    enum Event {
        NONE = 0x0,
        READ = 0x1,
        WRITE = 0x4,
    }
private:
    // Socket事件对应的类
    struct FdContext {
        typedef Mutex MutexType;
        /// 事件上下文
        struct EventContext {
             /// 事件执行的调度器: 事件由哪个调度器控制
            Scheduler* scheduler = nullptr;

            /// 事件协程
            Fiber::ptr fiber;
            /// 事件的回调函数
            std::function<void()> cb;            
        }

        /// 获取事件上下文类
        /// 根据传入的event，来判断最终的类型 读 or 写 or 读写
        // ?? 这里有没有可能一个fd上同时有读 写事件，那不就要返回两个??
        EventContext& getContext(Event event);  

        /// 重置该fd上的某个事件上下文
        void resetContext(EventContext& ctx);

        void triggerEvent(Event ctx);
        
        /// 每个fd上只会有两种事件，但是可以同时存在
        /// 读事件上下文
        EventContext read;
        /// 写事件上下文
        EventContext write;

        /// 事件关联的句柄
        int fd = 0;

        /// 当前的事件
        Event events = NONE;

        /// 事件的Mutex
        MutexType mutex;

    };
public:

    IOManager(size_t threads = 1, bool use_caller = true, const std::string& name = "");
    ~IOManager();

    /// 向被监听的的fb上增加某个事件
    int addEvent(int fd, Event event, std::function<void()> cb = nullptr);  // 只能用回调函数吗?
    /// 删除被监听的某个fd上的某种事件
    bool delEvent(int fd, Event event);
    bool cancelEvent(int fd, Event event);

    bool cancelAll(int fd); // 取消某个文件描述符下的所有事件

    static IOManager* GetThis();

protected:
    void tickle() override;
    bool stopping() override;
    void idle() override;

    /// 重置socket句柄上下文的容器大小
    void contextResize(size_t size);
private:
    int m_epfd = 0;     // epoll的文件句柄
    /// pipe 文件句柄
    int m_tickleFds[2]; // 有消息时，向管道中写入数据，
    /// 当前等待执行的(读写)事件数量
    std::atomic<size_t> m_pendingEventCount = {0};
    /// IOManager的Mutex
    RWMutexType m_mutex;

    /// socket事件上下文的容器!!!
    std::vector<FdContext*> m_fdContexts;
};


}


#endif
