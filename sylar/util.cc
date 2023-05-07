#include "util.h"


namespace sylar {
    
    pid_t GetThreadId() {
        return syscall(SYS_gettid);     // 利用系统调用获得线程ID
    }

    uint32_t GetFiberId() {
        return 0;
    }
}