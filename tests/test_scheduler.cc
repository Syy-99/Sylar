#include "../sylar/sylar.h"

static sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void test_fiber() {
    static int s_count = 5;
    SYLAR_LOG_INFO(g_logger) << "test in fiber s_count=" << s_count;

    sleep(1);
    if(--s_count >= 0) {
        sylar::Scheduler::GetThis()->schedule(&test_fiber, sylar::GetThreadId());
    }
}

int main() {
     SYLAR_LOG_INFO(g_logger) << "main";
    sylar::Scheduler sc;    // 创建一个调度器, 默认user_caller=true,说明该线程中的协程也是需要调度的

    // SYLAR_LOG_INFO(g_logger) << "schedule";
    // sc.schedule(&test_fiber);          // 将test_fiber加入调度, 此时调度器始终在run
    sc.start();  // 调度器开始工作
    // sleep(2);

    SYLAR_LOG_INFO(g_logger) << "schedule1";
    sc.schedule(&test_fiber);          // 将test_fiber加入调度, 此时调度器始终在run
    SYLAR_LOG_INFO(g_logger) << "schedule2";
    sc.stop();  // 调度器结束工作
    SYLAR_LOG_INFO(g_logger) << "over";
    return 0;
}