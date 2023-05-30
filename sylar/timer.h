/**
 * @file timer.h
 * @brief 定时器封装
 */
#ifndef __SYLAR_TIMER_H__
#define __SYLAR_TIMER_H__

#include <memroy>
#include <functional>
#include "mutex.h"


namespace sylar {

class TimerManager;
class Timer : public std::enable_shared_from_this<Timer> {
friend class TimerManager;
public:
    typedef std::shared_ptr<Timer> ptr;

private:
    /**
     * @brief 构造函数
     * @param[in] ms 定时器执行间隔时间
     * @param[in] cb 回调函数
     * @param[in] recurring 是否循环
     * @param[in] manager 定时器管理器
     */
    Timer(uint64_t ms, std::function<void()> cb, bool recurring, TimerManager* manager);        // Timer只能通过TimeManager创建

private:
    bool m_recurring = false;
    uint64_t m_ms = 0;      // 时间周期
    uint64_t m_next = 0;    // 该定时器任务的下一次的执行时间（绝对时间）
    std::function<void()> m_cb;
    TimerManager* m_manager = nullptr;

private:
    /**
     * @brief 定时器比较仿函数
     */
    struct Comparator {
        /**
         * @brief 比较定时器的智能指针的大小(按执行时间排序)
         * @param[in] lhs 定时器智能指针
         * @param[in] rhs 定时器智能指针
         */
        bool operator()(const Timer::ptr& lhs, const Timer::ptr& rhs) const;
    };
};

class TimerManager {
friend class Timer;
public:
    typedef std::shared_ptr<TimerManager> ptr;
    typedef RWMutex RWMutexType;        // 使用读写锁

    TimerManager();
    virtual ~TimerManager();

    Timer::ptr addTimer(uint64_t ms, std::function<void()> cb, bool recurring = false);

    /// 条件触发器
    Timer::ptr addConditionTimer(uint64_t ms, std::function<void()> cb,
                                ,std::weak_ptr<void> weak_cond      // !!!注意这里使用智能指针
                                ,bool recurring = false);
protected:
    /// 当有新的定时器插入到定时器的首部,执行该函数
    /// 为什么需要执行???有什么用????
    virtual void onTimerInsertedAtFront() = 0;

private:
    /// Mutex
    RWMutexType m_mutex;
    /// 定时器有序集合
    std::set<Timer::ptr, Timer::Comparator> m_timers;
//     /// 是否触发onTimerInsertedAtFront
//     bool m_tickled = false;
//     /// 上次执行时间
//     uint64_t m_previouseTime = 0;
};

}

#endif