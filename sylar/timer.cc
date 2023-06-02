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
    : m_recurring(recurring), m_ms(ms), m_cb(cb), m_manager(manager){
    m_next = sylar::GetCurrentMS() + m_ms;      
}   

bool Timer::cancel() {
    TimerManager::RWMutexType::WriteLock lock(m_manager->m_mutex);
    if (m_cb) {
        m_cb = nullptr;

        auto it = m_manager->m_timers.find(shared_from_this());
        m_manager->m_timers.erase(it);
        return true;
    }
    return false;
}

bool Timer::refresh() { // 重新设置时间
    TimerManager::RWMutexType::WriteLock lock(m_manager->m_mutex);
    if(!m_cb) {
        return false;
    }

    auto it = m_manager->m_timers.find(shared_from_this());
    if(it == m_manager->m_timers.end()) {
        return false;
    }
    m_manager->m_timers.erase(it);
    // 先删除再加入是为了重新排序

    m_next = sylar::GetCurrentMS() + m_ms;     // 基于当前时间重新设置
    m_manager->addTimer(shared_from_this(), lock);
    return true;
}

bool Timer::reset(uint64_t ms, bool from_now) {
    if(ms == m_ms && !from_now) {
        return true;
    }
    TimerManager::RWMutexType::WriteLock lock(m_manager->m_mutex);
    if(!m_cb) {
        return false;
    }
    auto it = m_manager->m_timers.find(shared_from_this());
    if(it == m_manager->m_timers.end()) {
        return false;
    }
    m_manager->m_timers.erase(it);

    uint64_t start = 0;
    if(from_now) {
        start = sylar::GetCurrentMS();
    } else {
        start = m_next - m_ms;      // 基于之前设置的时间
    }
    m_ms = ms;
    m_next = start + m_ms;
    it = m_manager->m_timers.insert(shared_from_this()).first;
    
    return true;
}

Timer::Timer(uint64_t next) : m_next(next) {

}

TimerManager::TimerManager() {
    m_previouseTime = sylar::GetCurrentMS();
}
TimerManager::~TimerManager() {

}

// void onTimerInsertedAtFront();


Timer::ptr TimerManager::addTimer(uint64_t ms, std::function<void()> cb, bool recurring) {
    Timer::ptr timer(new Timer(ms, cb, recurring, this));
    RWMutexType::WriteLock lock(m_mutex);
    addTimer(timer, lock);
    // auto it = m_timers.insert(timer).first;

    // bool at_front = (it == m_timers.begin());
    // lock.unlock();

    // if (at_front) {
    //     // 插入的定时器是最小的，则需要通知调度器唤醒epoll_wait ,重新设置超时时间
    //     onTimerInsertedAtFront();
    // }

    return timer;
}

static void OnTimer(std::weak_ptr<void> weak_cond, std::function<void()> cb) {
    std::shared_ptr<void> tmp = weak_cond.lock();   
    if (tmp) {  // 如果这个智能指针没释放，则说明条件满足
        cb();
    }
}

/// 条件触发器
Timer::ptr TimerManager::addConditionTimer(uint64_t ms, std::function<void()> cb
                                            ,std::weak_ptr<void> weak_cond      // !!!注意这里使用智能指针
                                            ,bool recurring){
    return addTimer(ms, std::bind(&OnTimer, weak_cond, cb), recurring);
}

uint64_t TimerManager::getNextTimer() {
    RWMutexType::ReadLock lock(m_mutex);
    m_tickled = false;      // ??? 这里是干什么
    if (m_timers.empty()) {
        return ~0ull;
    }

    const Timer::ptr& next = *m_timers.begin();
    uint64_t now_ms = sylar::GetCurrentMS();
    if (now_ms >= next->m_next) {
        return 0;
    } else {
        return next->m_next - now_ms;
    }
}

void TimerManager::listExpiredCb(std::vector<std::function<void()> >& cbs) {
    uint64_t now_ms = sylar::GetCurrentMS();
    std::vector<Timer::ptr> expired;
    {
        RWMutexType::ReadLock lock(m_mutex);
        if (m_timers.empty()) {
            return;
        }
    }
    RWMutexType::WriteLock lock(m_mutex);
    if(m_timers.empty()) {
        return;
    }

    // 检查是否没有改过时间，并且没有超时
    bool rollover = detectClockRollover(now_ms);
    if(!rollover && ((*m_timers.begin())->m_next > now_ms)) {
        return;
    }

    Timer::ptr now_timer(new Timer(now_ms));
    auto it = rollover ? m_timers.end() : m_timers.lower_bound(now_timer);
    while(it != m_timers.end() && (*it)->m_next == now_ms) {
        ++it;
    }

    expired.insert(expired.begin(), m_timers.begin(), it);  // 加入记录
    m_timers.erase(m_timers.begin(), it);   // 删除定时器
    cbs.reserve(expired.size());

    for (auto& timer : expired) {
        cbs.push_back(timer->m_cb);
        if (timer->m_recurring) {
            timer->m_next = now_ms + timer->m_ms;
            m_timers.insert(timer);
        }else{
            timer->m_cb = nullptr;  // 清除可能的智能指针引用计数 ?? 为什么这里要手动减1呢?感觉每必要呀
        }
    }
}   



bool TimerManager::hasTimer() {
    RWMutex::ReadLock lock(m_mutex);
    return !m_timers.empty();

}

void TimerManager::addTimer(Timer::ptr val, RWMutexType::WriteLock& lock) {
    auto it = m_timers.insert(val).first;

    bool at_front = (it == m_timers.begin()) && !m_tickled;
    if(at_front) {
        m_tickled = true;       // ???这里有什么用??
        //。实际实现时，只需要在onTimerInsertedAtFront()方法内执行一次tickle就行了，
        // tickle之后，epoll_wait会立即退出，并重新从TimerManager中获取最近的超时时间，这时拿到的超时时间就是新添加的最小定时器的超时时间了
    }
    lock.unlock();

    if (at_front) {
        // 插入的定时器是最小的，则需要通知调度器唤醒epoll_wait ,重新设置超时时间
        onTimerInsertedAtFront();
    }
}

bool TimerManager::detectClockRollover(uint64_t now_ms) {
    bool rollover = false;
    if(now_ms < m_previouseTime && 
      now_ms < (m_previouseTime - 60 * 60 * 1000)) {
        // 检查一下系统时间有没有被往回调。如果系统时间往回调了1个小时以上，那就触发全部定时器
        rollover = true;
    }
    m_previouseTime = now_ms;
    return rollover;
}


}