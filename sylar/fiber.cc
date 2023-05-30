#include "fiber.h"
#include "config.h"
#include "macro.h"
#include "log.h"
#include "scheduler.h"
#include <atomic>

namespace sylar {

static Logger::ptr g_logger = SYLAR_LOG_NAME("system");


/// 从0开始，确保原子性
static std::atomic<uint64_t> s_fiber_id {0};    
static std::atomic<uint64_t> s_fiber_count {0};


// 因为所有协程共用一个class代码，所以需要记录执行当前class代码的协程
/// 记录当前协程
static thread_local Fiber* t_fiber = nullptr;

/// 记录当前线程中的主协程
static thread_local Fiber::ptr t_threadFiber = nullptr;

/// 约定一个可用配置项： 定义栈大小
static ConfigVar<uint32_t>::ptr g_fiber_stack_size =
    Config::Lookup<uint32_t>("fiber.stack_size", 1024 * 1024, "fiber stack size");

/// 协程的栈的内存分配器
class MallocStackAllocator {
public:
    static void* Alloc(size_t size) {
        return malloc(size);
    }

    static void Dealloc(void* vp, size_t size) {
        return free(vp);
    }
};
using StackAllocator = MallocStackAllocator;


Fiber::Fiber() {      
    /// 第一个协程，即主协程的构造函数
    m_state = EXEC;
    SetThis(this);      

    /// 获得当前协程的上下文
    if(getcontext(&m_ctx)) {
        SYLAR_ASSERT2(false, "getcontext");
    }

    ++s_fiber_count;    // 总协程数量加1

    SYLAR_LOG_DEBUG(g_logger) << "Fiber::Fiber main";

}      

Fiber::Fiber(std::function<void()> cb, size_t stacksize, bool user_caller) : m_id(++s_fiber_id), m_cb(cb){
    /// 真正创建一个执行协程(子协程)
    ++s_fiber_count;

    m_stacksize = stacksize ? stacksize : g_fiber_stack_size->getValue();
    m_stack = StackAllocator::Alloc(m_stacksize);   // 分配内存空间给栈使用

    if(getcontext(&m_ctx)) {
        SYLAR_ASSERT2(false, "getcontext");
    }
    /// 设置上下文
    m_ctx.uc_link = nullptr;    // 该协程关联的上文
    // m_ctx.uc_link = &t_threadFiber->m_ctx;    // 该协程结束后，会回到主协程
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stacksize;

    if (user_caller) {
        makecontext(&m_ctx, &Fiber::CallerMainFunc, 0);   // 设置执行完后的回调函数
    } else {
        makecontext(&m_ctx, &Fiber::MainFunc, 0);   // 设置执行完后的回调函数
    }

    SYLAR_LOG_DEBUG(g_logger) << "Fiber::Fiber id=" << m_id;

}

Fiber::~Fiber() {
    --s_fiber_count;

    if (m_stack) {      
        SYLAR_ASSERT(m_state == TERM
                    || m_state == EXCEPT
                    || m_state == INIT);

        StackAllocator::Dealloc(m_stack, m_stacksize);
    } else { // 主协程没有栈，不需要回收
        // 确认是主协程
        SYLAR_ASSERT(!m_cb);
        SYLAR_ASSERT(m_state == EXEC);  // 主协程一直在运行? 主协程初始不是INIT状态吗？

        /// ????
        Fiber* cur = t_fiber;
        if(cur == this) {
            SetThis(nullptr);
        }
    }
    SYLAR_LOG_DEBUG(g_logger) << "Fiber::~Fiber id=" << m_id;
}

/// 重置协程执行函数，并设置状态 -> 减少内存分配和释放操作
void Fiber::reset(std::function<void()> cb) {
    SYLAR_ASSERT(m_stack); // 要求有栈

    SYLAR_ASSERT(m_state == TERM
                || m_state == EXCEPT
                || m_state == INIT);

    m_cb = cb;  // 重新设置执行函数

    if(getcontext(&m_ctx)) {
        SYLAR_ASSERT2(false, "getcontext");
    }

    m_ctx.uc_link = nullptr;
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stacksize;

    makecontext(&m_ctx, &Fiber::MainFunc, 0);   // 设置执行完后的回调函数
    m_state = INIT;
}

/// 将当前协程切换到运行态执行
void Fiber::swapIn() {
    SetThis(this);

    SYLAR_ASSERT(m_state != EXEC);
    m_state = EXEC;

    /// 交换主协程（正在运行）和工作协程的上下文，使得工作线程开始运行
    // if(swapcontext(&t_threadFiber->m_ctx, &m_ctx)) {

    // 和协程调度器的主协程交换
    if(swapcontext(&Scheduler::GetMainFiber()->m_ctx, &m_ctx)) {
        SYLAR_ASSERT2(false, "swapcontext");
    }
    

}


/// 将当前协程切换到后台, 运行主协程
void Fiber::back() {
    SetThis(t_threadFiber.get());
    if(swapcontext(&m_ctx, &t_threadFiber->m_ctx)) {
        SYLAR_ASSERT2(false, "swapcontext");
    }
}

/// 将当前协程切换到后台, 运行主协程
void Fiber::swapOut() {
    SetThis(Scheduler::GetMainFiber());
    if(swapcontext(&m_ctx, &Scheduler::GetMainFiber()->m_ctx)) {
        SYLAR_ASSERT2(false, "swapcontext");
    }
}

void Fiber::call() {
    SetThis(this);
    m_state = EXEC;
    if(swapcontext(&t_threadFiber->m_ctx, &m_ctx)) {
        SYLAR_ASSERT2(false, "swapcontext");
    }
}

/// 设置当前运行的协程
void Fiber::SetThis(Fiber* f) {
    t_fiber = f;
}

/// 返回当前正在执行的协程
Fiber::ptr Fiber::GetThis() {
    if(t_fiber) {
        return t_fiber->shared_from_this(); // 返回当前协程的智能指针
    }

    Fiber::ptr main_fiber(new Fiber);     // 初始化主协程
    SYLAR_ASSERT(t_fiber == main_fiber.get());  // 判断是否设置生效
    t_threadFiber = main_fiber;     // 保存主协程
    return t_fiber->shared_from_this();
}

/// 将当前协程切换到后台,设置为READY状态, 
void Fiber::YieldToReady() {
    Fiber::ptr cur = GetThis();
    SYLAR_ASSERT(cur->m_state == EXEC);

    cur->m_state = READY;
    cur->swapOut();     // 切换到主协程
}
/// 将当前协程切换到后台,并设置为HOLD状态
void Fiber::YieldToHold() {
    Fiber::ptr cur = GetThis();
    SYLAR_ASSERT(cur->m_state == EXEC);

    cur->m_state = HOLD;
    cur->swapOut();     // 切换到主协程
}
/// 总协程数量
uint64_t Fiber::TotalFibers() {
    return s_fiber_count;
}

/// 协程执行函数
void Fiber::MainFunc() {
    Fiber::ptr cur = GetThis();      // 增加引用计数，导致对象无法被释放
    SYLAR_ASSERT(cur);

    try {
        cur->m_cb();            // 执行协程的函数
        cur->m_cb = nullptr;    // 执行完就清空
        cur->m_state = TERM;    // 执行完状态就改变
    } catch (std::exception& ex) {
        cur->m_state = EXCEPT;
        SYLAR_LOG_ERROR(g_logger) << "Fiber Except: " << ex.what()
                                  << " fiber_id=" << cur->getId()
                                  << std::endl
                                  << sylar::BacktraceToString();
    } catch (...) {
        cur->m_state = EXCEPT;
        SYLAR_LOG_ERROR(g_logger) << "Fiber Except"
                                << " fiber_id=" << cur->getId()
                                << std::endl
                                << sylar::BacktraceToString();
        }

    auto raw_ptr = cur.get();
    cur.reset();        // 释放这个智能指针
    raw_ptr->swapOut();     // 如果不是user_caller, 则会和main协程切换

    SYLAR_ASSERT2(false, "never reach fiber_id=" + std::to_string(raw_ptr->getId()));
}

void Fiber::CallerMainFunc() {
    Fiber::ptr cur = GetThis();
    SYLAR_ASSERT(cur);
    try {
        cur->m_cb();
        cur->m_cb = nullptr;
        cur->m_state = TERM;
    } catch (std::exception& ex) {
        cur->m_state = EXCEPT;
        SYLAR_LOG_ERROR(g_logger) << "Fiber Except: " << ex.what()
            << " fiber_id=" << cur->getId()
            << std::endl
            << sylar::BacktraceToString();
    } catch (...) {
        cur->m_state = EXCEPT;
        SYLAR_LOG_ERROR(g_logger) << "Fiber Except"
            << " fiber_id=" << cur->getId()
            << std::endl
            << sylar::BacktraceToString();
    }

    auto raw_ptr = cur.get();
    cur.reset();
    raw_ptr->back();    // 如果是user_caller, 调度协程不能自己和自己切换，需要和主协程切换

    SYLAR_ASSERT2(false, "never reach fiber_id=" + std::to_string(raw_ptr->getId()));

}

uint64_t Fiber::GetFiberId() {
    if (t_fiber) {
        return t_fiber->m_id;
    }

    return 0;
}

}