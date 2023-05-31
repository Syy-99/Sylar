#include "util.h"
#include <execinfo.h>
#include <sys/time.h>

#include "log.h"
#include "fiber.h"

namespace sylar {

sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");     // 获得日志器
    
pid_t GetThreadId() {
    return syscall(SYS_gettid);     // 利用系统调用获得线程ID
}

uint32_t GetFiberId() {
    return sylar::Fiber::GetFiberId();
}

void Backtrace(std::vector<std::string>& bt, int size, int skip)  {
    void** array = (void**)malloc((sizeof(void*) * size));      // 用堆空间而不用栈空间(协程的栈空间小，可以快速切换)
    size_t s = ::backtrace(array, size);        // 返回真实层数

    char **strings = ::backtrace_symbols(array, s);
    if(strings == NULL) {
        SYLAR_LOG_ERROR(g_logger) << "backtrace_synbols error";
        return;
    }

    for(size_t i = skip; i < s; ++i) {
        bt.push_back(strings[i]);     // 将调用栈的名字保存到vecotr中
    }

    free(strings);
    free(array);
}

std::string BacktraceToString(int size, int skip, const std::string& prefix) {
    std::vector<std::string> bt;
    Backtrace(bt, size, skip);
    std::stringstream ss;

    for(size_t i = 0; i < bt.size(); ++i) {
        ss << prefix << bt[i] << std::endl;
    }

    return ss.str();
}


uint64_t GetCurrentMS() {
    struct timeval tv;
    gettimeofday(&tv, NULL);

   return tv.tv_sec * 1000ul  + tv.tv_usec / 1000;

}

uint64_t GetCurrentUS() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 * 1000ul  + tv.tv_usec;

}

}