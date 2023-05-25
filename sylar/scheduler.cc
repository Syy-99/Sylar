#include "scheduler.h"
#include "log.h"
#include "macro.h"

namespace sylar{

static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

static thread_local Scheduler* t_scheduler = nullptr;       // 协程调度器的指针
static thread_local Fiber* t_scheduler_fiber = nullptr;     // 协程调度器的主协程的指针

Scheduler::Scheduler(size_t threads, bool use_caller, const std::string& name) : m_name(name){
    SYLAR_ASSERT(threads > 0);

    // 这个地方的关键点在于，是否把创建协程调度器的线程放到协程调度器管理的线程池中。
    // 如果不放入，那这个线程专职协程调度；如果放的话，那就要把协程调度器封装到一个协程中，称之为主协程或协程调度器协程。
    if (use_caller) { //???
        sylar::Fiber::GetThis(); // 如果线程没有协程，则会初始化一个主协程
        --threads;  // 该线性运行调度器，所以不能放在线程池中

        SYLAR_ASSERT(GetThis() == nullptr);     // 确保只有一个调度器
        t_scheduler = this;         // 设置当前线程的协程调度器

        m_rootFiber.reset(new Fiber(std::bind(&Scheduler::run, this),0, true));    // 在当前线程中，创建一个调度协程，执行run

        sylar::Thread::SetName(m_name);

        t_scheduler_fiber = m_rootFiber.get();

        m_rootThread = sylar::GetThreadId();   // 获得主线程id
        m_threadIds.push_back(m_rootThread);    // 记录线程id,相当于加入线程池
    } else {
        m_rootThread = -1;
        //?? 这里没有创建协程调度器，原因可能是该线程只是作为线程池的对象，并不执行协程调度
    }

    m_threadCount = threads;
}

Scheduler:: ~Scheduler() {
    SYLAR_ASSERT(m_stopping);

    if(GetThis() == this) {
        t_scheduler = nullptr;
    }

}


Scheduler* Scheduler::GetThis() {
    return t_scheduler;
}

/// 返回当前协程调度器的调度协程
Fiber* Scheduler::GetMainFiber() {
    return t_scheduler_fiber;
}

/// 启动协程调度器
void Scheduler::start() {
    MutexType::Lock lock(m_mutex);

    if (!m_stopping) {
        return;
    }

    m_stopping = false;
    SYLAR_ASSERT(m_threads.empty());

    // 创建线程池
    m_threads.resize(m_threadCount);
    for(size_t i = 0; i < m_threadCount; ++i) {
        m_threads[i].reset(new Thread(std::bind(&Scheduler::run, this)      // 线程池中的线程都执行run
                            , m_name + "_" + std::to_string(i)));
        
        m_threadIds.push_back(m_threads[i]->getId());
    }
    lock.unlock();  // 手动解锁，因为该函数会跳到run中，导致lock没有释放

    if(m_rootFiber) {
       //m_rootFiber->swapIn();
       m_rootFiber->call();     // 线程中的调度协程和主协程，表示现在开始运行调度协程
                                // 调度协程中的函数是run(), 所以实际上会执行run函数
       SYLAR_LOG_INFO(g_logger) << "call out " << m_rootFiber->getState();
    }

}

/// 停止协程调度器
void Scheduler::stop() {
    m_autoStop = true;

    if (m_rootFiber 
        && m_threadCount == 0
         && (m_rootFiber->getState() == Fiber::TERM 
             ||m_rootFiber->getState() == Fiber::INIT)) {
        SYLAR_LOG_INFO(g_logger) << this << " stopped";
        m_stopping = true;         

        if(stopping()) {
            return;
        }      
    }

    //??? 这里的目的是什么
    if (m_rootThread != -1) {   
        SYLAR_ASSERT(GetThis() == this);    // 只能在协程调度器所属的线程中停止??? 和user_caller有关
    }  else {
        SYLAR_ASSERT(GetThis() != this);
    }

    m_stopping = true;
    for(size_t i = 0; i < m_threadCount; ++i) {
        tickle();   // 唤醒线程， 结束工作, 这里怎么唤醒就结束了??
        SYLAR_LOG_INFO(g_logger) << this << " stopped1";
    }

    /// ????如果是主协程
    if(m_rootFiber) {
        tickle();
        SYLAR_LOG_INFO(g_logger) << this << " stopped2";
    }

    if(stopping()) {
        return;
    }
}

void Scheduler::setThis() {
    t_scheduler = this;
}

// run是一个静态成员函数
// ???逻辑需要梳理
void Scheduler::run() {     
    SYLAR_LOG_DEBUG(g_logger) << m_name << " run " << t_scheduler_fiber->getId();
    // return;
    // Fiber::GetThis();
    setThis();    // 设置当前执行的调度器 ?? 调用的时候Scheduler对象在哪?

    if (sylar::GetThreadId() != m_rootThread) {    
        t_scheduler_fiber = Fiber::GetThis().get();   
    }

    Fiber::ptr idle_fiber(new Fiber(std::bind(&Scheduler::idle, this)));
    Fiber::ptr cb_fiber;  // ??

    FiberAndThread ft;
    while(true) {
        ft.reset();
        bool tickle_me = false;
        bool is_active = false;
        {   // 从协程队列中取出协程执行
            MutexType::Lock lock(m_mutex);
            auto it = m_fibers.begin();
            while(it != m_fibers.end()) {
                if (it->thread != -1 && it->thread != sylar::GetThreadId()) {
                    // 该协程指定了线程,且不是当前线程
                    ++it;
                    tickle_me = true;   // 可能是无序的???被其他线程调度 如何理解?
                    continue;
                }

                SYLAR_ASSERT(it->fiber || it->cb);
                if (it->fiber && it->fiber->getState() == Fiber::EXEC) {    // ? 在队列中的协程怎么会在执行状态呢?
                    ++it;
                    continue;
                }

                ft = *it;
                m_fibers.erase(it++);
                ++m_activeThreadCount;
                is_active = true;
                break;
            }
        }

        if (tickle_me) {
            tickle();
        }

        if (ft.fiber  && ft.fiber->getState() != Fiber::TERM 
                      && ft.fiber->getState() != Fiber::EXCEPT) {
            ft.fiber->swapIn();
            --m_activeThreadCount;

            if (ft.fiber->getState() == Fiber::READY) {
                schedule(ft.fiber);
            } else if(ft.fiber->getState() != Fiber::TERM
                      && ft.fiber->getState() != Fiber::EXCEPT) {
                ft.fiber->m_state = Fiber::HOLD;    // 让出执行
            }
            ft.reset();
        } else if (ft.cb) {
            if (cb_fiber) {
                cb_fiber->reset(ft.cb);
            } else {
                cb_fiber.reset(new Fiber(ft.cb));
            }
            ft.reset();

            cb_fiber->swapIn();
            --m_activeThreadCount;

            if(cb_fiber->getState() == Fiber::READY) {
                schedule(cb_fiber);  // 还没结束，继续调用
                cb_fiber.reset();
            }  else if(cb_fiber->getState() == Fiber::EXCEPT
                    || cb_fiber->getState() == Fiber::TERM) {
                cb_fiber->reset(nullptr);   // 释放
            } else {
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
    SYLAR_LOG_INFO(g_logger) << "tickle";
}

bool Scheduler::stopping() {
    MutexType::Lock lock(m_mutex);
    return m_autoStop && m_stopping
        && m_fibers.empty() && m_activeThreadCount == 0;
}

void Scheduler::idle() {
      SYLAR_LOG_INFO(g_logger) << "idle";
}

}