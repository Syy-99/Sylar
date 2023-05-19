#include "fiber.h"
#include "config.h"
#include "macro.h"
#include "log.h"
#include <atomic>

namespace sylar {

static Logger::ptr g_logger = SYLAR_LOG_NAME("system");


/// 从0开始，确保原子性
static std::atomic<uint64_t> s_fiber_id {0};    
static std::atomic<uint64_t> s_fiber_count {0};

/// 记录当前协程自己
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
    m_stack = EXEC;
    SetThis(this);      //?????

    /// 获得当前协程的上下文
    if(getcontext(&m_ctx)) {
        SYLAR_ASSERT2(false, "getcontext");
    }

    ++s_fiber_count;    // 总协程数量加1

    // SYLAR_LOG_DEBUG(g_logger) << "Fiber::Fiber main";

}      

Fiber(std::function<void()> cb, size_t stacksize = 0) : m_id(++s_fiber_id), m_cb(cb){
    /// 真正创建一个执行协程(子协程)
    ++s_fiber_count;

    m_stacksize = stacksize ? stacksize : g_fiber_stack_size->getValue();
    m_stack = StackAllocator::Alloc(m_stacksize);   // 分配内存空间给栈使用

    if(getcontext(&m_ctx)) {
        SYLAR_ASSERT2(false, "getcontext");
    }
    /// 设置上下文
    m_ctx.uc_link = nullptr;    // 该协程关联的上文
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stacksize;

    makecontext(&m_ctx, &Fiber::MainFunc, 0);   // 设置执行的函数


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
}

/// 重置协程执行函数，并设置状态 -> 减少内存分配和释放操作
void reset(std::function<void() cb); 

/// 将当前协程切换到运行态执行
void swapIn();
/// 将当前协程切换到后台
void swapOut();

/// 设置当前运行的协程
void Fiber::SetThis(Fiber* f) {
    t_fiber = f;
}

/// 返回当前所在的协程
static Fiber::ptr GetThis();

/// 将当前协程切换到后台,并设置为READY状态
static void YieldToReady();
/// 将当前协程切换到后台,并设置为HOLD状态
static void YieldToHold();
/// 总协程数量
static uint64_t TotalFibers();

/// 协程执行函数， 执行完成会返回到线程的主协程
static void MainFunc();
}