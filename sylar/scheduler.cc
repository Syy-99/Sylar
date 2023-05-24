#include "scheduler.h"
#include "log.h"
#include "macro.h"

namespace {

static sylar::logger::ptr g_logger = SYLAR_LOG_NAME("system");

static thread_local Scheduler* t_scheduler = nullptr;   // 协程调度器的指针
static thread_local Fiber* t_scheduler_fiber = nullptr; // 协程调度器的主协程的指针

Scheduler::Scheduler(size_t threads, bool use_caller = true, const std::string& name) : m_name(name){
    SYLAR_ASSERT(threads > 0);

    // 这个地方的关键点在于，是否把创建协程调度器的线程放到协程调度器管理的线程池中。
    // 如果不放入，那这个线程专职协程调度；如果放的话，那就要把协程调度器封装到一个协程中，称之为主协程或协程调度器协程。
    if (use_caller) { //???
        sylar::Fiber::GetThis(); // 如果线程没有协程，则会初始化一个主协程
        --threads;

        SYLAR_ASSERT(GetThis() == nullptr);     // 确保当前线程只有一个调度器
        t_scheduler = this;         // 设置当前线程的协程调度器

        m_rootFiber.reset(new Fiber(std::bind(&Scheduler::run, this), 0, true));    // 创建一个调度协程，负责调度器的运行

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
    return t_scheduler_fiber
}

/// 启动协程调度器
void Scheduler::start() {
    MutexType::Lock lock(m_mutex);

    if (!m_stopping) {
        return;
    }

    m_stopping = false;
    SYLAR_ASSERT(m_threads.empty());

    m_threads.resize(m_threadCount);
    for(size_t i = 0; i < m_threadCount; ++i) {
        m_threads[i].reset(new Thread(std::bind(&Scheduler::run, this)      // 所有线程都执行run函数
                            , m_name + "_" + std::to_string(i)));
        
        m_threadIds.push_back(m_threads[i]->getId());
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

    bool exit_on_this_fiber = false;
    //??? 这里的目的是什么
    if (m_rootThread != -1) {   
        SYLAR_ASSERT(GetThis() == this);    // 只能在协程调度器所属的线程中停止??? 和user_caller有关
    }  else {
        SYLAR_ASSERT(GetThis() != this);
    }

    m_stopping = true;
    for(size_t i = 0; i < m_threadCount; ++i) {
        tickle();   // 唤醒线程， 结束工作, 这里怎么唤醒就结束了??
    }

    /// ????如果是主协程
    if(m_rootFiber) {
        tickle();
    }

    if(stopping()) {
        return;
    }
}

void Scheduler::setThis() {
    t_scheduler = this;
}

void schedule::run() {
    setThis();    // 设置当前执行的调度器

    if (sylar::GetThreadId() != m_rootThread) {    
        t_scheduler_fiber = Fiber::GetThis().get();   
    }

    Fiber::ptr idle_fiber(new Fiber(std::bind(&Scheduler::idle, this)));


}

}