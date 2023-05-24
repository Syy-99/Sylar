/**
 * @file scheduler.h
 * @brief 协程调度器封装
 */

#ifndef __SYLAR_SCHEDULER_H__
#define __SYLAR_SCHEDULER_H__

#include <memory>
#include "fiber.h"
#include "mutex.h"
#include "thread.h"

namespace sylar {

/**
 * @brief 协程调度器
 * @details 封装的是N-M的协程调度器
 *          内部有一个线程池,支持协程在线程池里面切换
 */
class Scheduler {
public:
    typedef std::shared_ptr<Scheduler> ptr;
    typedef Mutex MutexType;        // 使用互斥锁

    /**
     * @brief 构造函数
     * @param[in] threads 线程数量
     * @param[in] use_caller 是否使用当前调用线程 ??
     * @param[in] name 协程调度器名称
     */
    Scheduler(size_t threads = 1, bool use_caller = true, const std::string& name = "")
    virtual ~Scheduler();

    const std::string getName() const { return m_name; }

    static Scheduler* GetThis();
    /// 返回当前协程调度器的调度协程
    static Fiber* GetMainFiber();

    /// 启动协程调度器
    void start();
    /// 停止协程调度器
    void stop();

    /// 调度一个协程
    template<class FiberorCb>
    void schedule(FiberOrCb fc, int thread = -1) {
        bool need_tickle = false;
        {
            MutexType::Lock lock(m_mutex);
            need_tickle = scheduleNoLock(fc ,thread);
        }

        if (need_tickle) {
            tickle();
        }
    }

    /// 批量调度协程：确保连续的任务按连续的顺序执行
    template<class InputIterator>
    void schedule(InputIterator begin, InputIterator end) {
        bool need_tickle = false;
        {
            MutexType::Lock lock(m_mutex);
            while(begin != end) {
                need_tickle = scheduleNoLock(&*begin, -1) || need_tickle;   // 为什么这里用地址?
                ++begin;
            }
        }
        if(need_tickle) {   // 之前协程队列为空，现在不为空，则需要通知协程调度器可以执行调度工作
            tickle();
        }
    }

protected:
    /// 通知协程调度器有任务了
    virtual viud tickle();
private:
    /// 协程调度启动(无锁)
    template<class FiberorCb>
    void schedule(FiberOrCb fc, int thread) {
        bool need_tickle = m_fibers.empty();
        FiberAndThread ft(fc, thread);
        if (ft.fiber || ft.cb) {    // 这里的判断有意义吗? 不就只有这两种类型
            m_fibers.push_back(ft);
        }
        return need_tickle; 
        // 返回ture: 说明之前所有工作都在执行，现在有新的任务到来
        // 返回false: 说明本身就有任务等待执行，然后呢???
    }
private:
    // 协程/函数/线程组
    struct FiberAndThread {
        /// 协程
        Fiber::ptr fiber;
        /// 协程执行函数
        std::function<void()> cb;
        /// 线程id
        int thread;

        // 该f希望在线程id=thr的线程中运行
        FiberAndThread(Fiber::ptr f, int thr) : fiber(f), thread(thr) {}

        /// ????
        FiberAndThread(Fiber::ptr* f, int thr) : thread(thr) {
            fiber.swap(&f); // 为什么前一个构造函数不需要swap呢?

            /// 这难道不是程序员规定的吗: 传递指针就是swap,不传就普通拷贝
            /// 但是，那第一个构造函数会导致引用计数增加，难道就不需要考虑可能的问题吗?
        }

        FiberAndThread(std::function<void()> f, int thr)
            :cb(f), thread(thr) {
        }

        FiberAndThread(std::function<void()>* f, int thr)
            :thread(thr) {
            cb.swap(*f);
        }

        /// 提供给STL容器使用
        FiberAndThread() : thread(-1) {

        }

        void reset() {
            fiber = nullptr;
            cb = nullptr;
            thread = -1;
        }
    }
private:
    /// Mutex
    MutexType m_mutex;
    /// 线程池
    std::vector<Thread::ptr> m_threads;
    /// 待执行的协程队列
    std::list<FiberAndThread> m_fibers;   // ?? 为什们用list
    std::string m_name;
}

}   // sylar
#endif
