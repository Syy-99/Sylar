/**
 * @file fiber.h
 * @brief 协程封装
 */
#ifndef __SYLAR_FIBER_H__
#define __SYLAR_FIBER_H__

#include <memory>
#include <functional>
#include<ucontext.h>


namespace sylar{     

class Fiber : public std::enable_shared_from_this<Fiber> { // enable_shared_from_this直接获得当前类的智能指针
public:
    typedef std::shared_ptr<Fiber> ptr;

    /**
     * @brief 协程状态
     */
    enum State {
        /// 初始化状态
        INIT,
        /// 暂停状态
        HOLD,
        /// 执行中状态
        EXEC,
        /// 结束状态
        TERM,
        /// 可执行状态
        READY,
        /// 异常状态
        EXCEPT
    };

private:

    Fiber();        // 私有，不允许直接构造，因为智能指针对象必须在堆上

public:

    Fiber(std::function<void()> cb, size_t stacksize = 0);
    ~Fiber();

    /// 重置协程执行函数，并设置状态 -> 减少内存分配和释放操作
    void reset(std::function<void()> cb); 

    /// 将当前协程切换到运行态执行
    void swapIn();
    /// 将当前协程切换到后台
    void swapOut();
    
    /// 设置当前协程
    static void SetThis(Fiber* f);
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

    /// 获得当前协程id
    static uint64_t GetFiberId();

private:
    uint64_t m_id = 0;          // 协程id
    uint32_t m_stacksize = 0;   // 协程运行栈大小
    State m_state = INIT;       // 协程状态
    ucontext_t m_ctx;           // 协程上下文
    void* m_stack = nullptr;    // 协程运行栈指针
    std::function<void()> m_cb; // 协程运行函数
};


}
#endif