#include "scheduler.h"
#include "log.h"
#include "macro.h"
#include "hook.h"
namespace sylar {

static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

static thread_local Scheduler* t_scheduler = nullptr;
static thread_local Fiber* t_fiber = nullptr;

Scheduler::Scheduler(size_t threads, bool use_caller, const std::string& name)
    :m_name(name) {
    SYLAR_ASSERT(threads > 0);

    if(use_caller) {
        sylar::Fiber::GetThis();
        --threads;

        SYLAR_ASSERT(GetThis() == nullptr);
        t_scheduler = this;

        m_rootFiber.reset(new Fiber(std::bind(&Scheduler::run, this), 0, true));
        sylar::Thread::SetName(m_name);

        t_fiber = m_rootFiber.get();
        m_rootThread = sylar::GetThreadId();
        m_threadIds.push_back(m_rootThread);
    } else {
        m_rootThread = -1;
    }
    m_threadCount = threads;
}

Scheduler::~Scheduler() {
    SYLAR_ASSERT(m_stopping);
    if(GetThis() == this) {
        t_scheduler = nullptr;
    }
}

Scheduler* Scheduler::GetThis() {
    return t_scheduler;
}

Fiber* Scheduler::GetMainFiber() {
    return t_fiber;
}

void Scheduler::start() {
    MutexType::Lock lock(m_mutex);
    if(!m_stopping) {
        return;
    }
    m_stopping = false;
    SYLAR_ASSERT(m_threads.empty());

    m_threads.resize(m_threadCount);
    for(size_t i = 0; i < m_threadCount; ++i) {
        m_threads[i].reset(new Thread(std::bind(&Scheduler::run, this)
                            , m_name + "_" + std::to_string(i)));
        m_threadIds.push_back(m_threads[i]->getId());
    }
    lock.unlock();

    //if(m_rootFiber) {
    //    //m_rootFiber->swapIn();
    //    m_rootFiber->call();
    //    SYLAR_LOG_INFO(g_logger) << "call out " << m_rootFiber->getState();
    //}
}

void Scheduler::stop() {
    SYLAR_LOG_INFO(g_logger) << " stop";
    m_autoStop = true;
    if(m_rootFiber
            && m_threadCount == 0
            && (m_rootFiber->getState() == Fiber::TERM
                || m_rootFiber->getState() == Fiber::INIT)) {
        SYLAR_LOG_INFO(g_logger) << this << " stopped";
        m_stopping = true;

        if(stopping()) {
            return;
        }
    }

    //bool exit_on_this_fiber = false;
    if(m_rootThread != -1) {
        SYLAR_ASSERT(GetThis() == this);
    } else {
        SYLAR_ASSERT(GetThis() != this);
    }

    m_stopping = true;
    for(size_t i = 0; i < m_threadCount; ++i) {
        tickle();       // 结合IO协程调度，那么这里的tickle有什么用，又没有事件发生??
    }

    if(m_rootFiber) {
        tickle();
    }

    if(m_rootFiber) {
        //while(!stopping()) {
        //    if(m_rootFiber->getState() == Fiber::TERM
        //            || m_rootFiber->getState() == Fiber::EXCEPT) {
        //        m_rootFiber.reset(new Fiber(std::bind(&Scheduler::run, this), 0, true));
        //        SYLAR_LOG_INFO(g_logger) << " root fiber is term, reset";
        //        t_fiber = m_rootFiber.get();
        //    }
        //    m_rootFiber->call();
        //}
        if(!stopping()) {
            m_rootFiber->call();    
        }
    }

    std::vector<Thread::ptr> thrs;
    {
        MutexType::Lock lock(m_mutex);
        thrs.swap(m_threads);
    }

    for(auto& i : thrs) {
        i->join();
    }
    //if(exit_on_this_fiber) {
    //}
}

void Scheduler::setThis() {
    t_scheduler = this;
}

void Scheduler::run() {
    SYLAR_LOG_INFO(g_logger) << "run";
    // 所有线程都默认启动hook
    set_hook_enable(true);

    setThis();
    if(sylar::GetThreadId() != m_rootThread) {
        t_fiber = Fiber::GetThis().get();       // 创建主协程
    }

    Fiber::ptr idle_fiber(new Fiber(std::bind(&Scheduler::idle, this)));    // 创建空闲协程
    Fiber::ptr cb_fiber;

    FiberAndThread ft;
    while(true) {
        ft.reset();
        bool tickle_me = false;
        bool is_active = false;
        {
            MutexType::Lock lock(m_mutex);
            auto it = m_fibers.begin();
            // SYLAR_LOG_INFO(g_logger) << "before find " << m_fibers.size();
            while(it != m_fibers.end()) {
                // SYLAR_LOG_INFO(g_logger) << "find!! " << m_fibers.size();
                if(it->thread != -1 && it->thread != sylar::GetThreadId()) {
                    ++it;
                    tickle_me = true;
                    continue;
                }

                SYLAR_ASSERT(it->fiber || it->cb);
                if(it->fiber && it->fiber->getState() == Fiber::EXEC) {
                    ++it;
                    continue;
                }

                ft = *it;
                m_fibers.erase(it);
                // SYLAR_LOG_INFO(g_logger) << "find!!2 " << m_fibers.size();
                ++m_activeThreadCount;
                is_active = true;
                break;
            }
        }

        if(tickle_me) {
            tickle();
        }

        if(ft.fiber && (ft.fiber->getState() != Fiber::TERM
                        && ft.fiber->getState() != Fiber::EXCEPT)) {
            ft.fiber->swapIn();
            --m_activeThreadCount;

            if(ft.fiber->getState() == Fiber::READY) {
                schedule(ft.fiber);
            } else if(ft.fiber->getState() != Fiber::TERM
                    && ft.fiber->getState() != Fiber::EXCEPT) {
                ft.fiber->m_state = Fiber::HOLD;
            }
            ft.reset();
        } else if(ft.cb) {
            if(cb_fiber) {
                cb_fiber->reset(ft.cb);     // 设置这个协程需要执行的代码
            } else {
                cb_fiber.reset(new Fiber(ft.cb));     // 这里是否意味着，实际上每个线程中只有一个工作协程??
                // ?? 那这里的协程的优点还是没有体现出来呀?
            }
            ft.reset();
            cb_fiber->swapIn();
            --m_activeThreadCount;
            if(cb_fiber->getState() == Fiber::READY) {
                schedule(cb_fiber);
                cb_fiber.reset();
            } else if(cb_fiber->getState() == Fiber::EXCEPT
                    || cb_fiber->getState() == Fiber::TERM) {
                cb_fiber->reset(nullptr);
            } else {//if(cb_fiber->getState() != Fiber::TERM) {
                cb_fiber->m_state = Fiber::HOLD;
                cb_fiber.reset();
            }
        } else {
            if(is_active) {
                --m_activeThreadCount;
                continue;
            }
            if(idle_fiber->getState() == Fiber::TERM) {
                SYLAR_LOG_INFO(g_logger) << "idle fiber term";
                tickle();   // 某个idle结束，说明已经没有可执行的任务了，则可以立刻唤醒其他陷入epoll_wait的线程
                break;
            }

            ++m_idleThreadCount;
            idle_fiber->swapIn();
            --m_idleThreadCount;
            if(idle_fiber->getState() != Fiber::TERM
                    && idle_fiber->getState() != Fiber::EXCEPT) {
                idle_fiber->m_state = Fiber::HOLD;
            }
        }
    }
}



void Scheduler::tickle() {
    SYLAR_LOG_INFO(g_logger) << "tickle ";
}

bool Scheduler::stopping() {
    MutexType::Lock lock(m_mutex);
    // SYLAR_LOG_INFO(g_logger) << m_autoStop << " " << m_stopping 
    //                 << " " << m_fibers.size() << " " << m_activeThreadCount;
    return m_autoStop && m_stopping 
        && m_fibers.empty() && m_activeThreadCount == 0;
}

void Scheduler::idle() {
    SYLAR_LOG_INFO(g_logger) << "idle";
    while(!stopping()) {
        //  SYLAR_LOG_INFO(g_logger) << "free idle";
        sylar::Fiber::YieldToHold();        // 只是让出，而不退出
    }
}

}