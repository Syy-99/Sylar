#include "timer.h"
#include "util.h"

namespace sylar {

bool Timer::Comparator::operator()(const Timer::ptr& lhs, const Timer::ptr& rhs) const {
    if(!lhs && !rhs) {
        return false;
    }
    if(!lhs) {
        return true;
    }
    if(!rhs) {
        return false;
    }
    // 为什么会出现空的情况呢?


    if(lhs->m_next < rhs->m_next) { // 按执行时间近的排序
        return true;
    }
    if(rhs->m_next < lhs->m_next) {
        return false;
    }

    return lhs.get() < rhs.get();

}

Timer::Timer(uint64_t ms, std::function<void()> cb, 
             bool recurring, TimerManager* manager) 
    :m_ms(ms), m_cb(cb), m_recurring(recurring), m_manager(manager){
    m_next = sylar::GetCurrentMS() + m_ms;      
}   


TimerManager::TimerManager();
TimerManager~::TimerManager();

void onTimerInsertedAtFront();


Timer::ptr TimerManager::addTimer(uint64_t ms, std::function<void()> cb, bool recurring = false) {
    Timer::ptr timer(new Timer(ms, cb, recurring, this));
    RWMutexType::WriteLock lock(m_mutex);
    
    auto it = m_timers.insert(timer).first;

    bool at_front = (it == m_timers.begin());
    lock.unlock();

    if (at_front) {
        // 插入的定时器是最小的，则需要通知调度器
        onTimerInsertedAtFront();
    }
}

/// 条件触发器
Timer::ptr TimerManager::addConditionTimer(uint64_t ms, std::function<void()> cb,
                            ,std::weak_ptr<void> weak_cond      // !!!注意这里使用智能指针
                            ,bool recurring = false);

}