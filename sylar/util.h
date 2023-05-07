/**
 * @file util.h
 * @brief 常用的工具函数
 */
#ifndef __SYLAR_UTIL_H__
#define __SYLAR_UTIL_H__

#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <stdint.h>

namespace sylar {
    // 获得线程id
    pid_t GetThreadId();
    // 获得协程id
    uint32_t GetFiberId();

}


#endif