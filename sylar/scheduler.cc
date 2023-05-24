#include "scheduler.h"
#include "log.h"

namespace {

static sylar::logger::ptr g_logger = SYLAR_LOG_NAME("system");

Scheduler::Scheduler(size_t threads = 1, bool use_caller = true, const std::string& name)

Scheduler:: ~Scheduler()


Scheduler* Scheduler::GetThis();

/// 返回当前协程调度器的调度协程
Fiber* Scheduler::GetMainFiber();

/// 启动协程调度器
void Scheduler::start();
/// 停止协程调度器
void Scheduler::stop();
}