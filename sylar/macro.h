/**
 * @file macro.h
 * @brief 常用宏的封装
 */

#ifndef __SYLAR_MACRO_H__
#define __SYLAR_MACRO_H__

#include <string.h>
#include <assert.h>
#include "log.h"
#include "util.h"

/// 编译器优化
/*
如果存在跳转指令，那么预先取出的指令就无用了。
cpu在执行当前指令时，从内存中取出了当前指令的下一条指令。
执行完当前指令后，cpu发现不是要执行下一条指令,而是执行offset偏移处的指令。
cpu只能重新从内存中取出offset偏移处的指令。
因此，跳转指令会降低流水线的效率，也就是降低cpu的效率。
*/
#if defined __GNUC__ || defined __llvm__
/// LIKCLY 宏的封装, 告诉编译器优化,条件大概率成立
#   define SYLAR_LIKELY(x)       __builtin_expect(!!(x), 1)     // x更有可能是真的
/// LIKCLY 宏的封装, 告诉编译器优化,条件大概率不成立
#   define SYLAR_UNLIKELY(x)     __builtin_expect(!!(x), 0)     // x更有可能是假的
#else
#   define SYLAR_LIKELY(x)      (x)
#   define SYLAR_UNLIKELY(x)      (x)
#endif

#define SYLAR_ASSERT(x) \
    if(SYLAR_UNLIKELY(!(x))) { \
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "ASSERTION: " #x \
            << "\nbacktrace:\n" \
            << sylar::BacktraceToString(100, 2, "    "); \
        assert(x); \
    } 

#define SYLAR_ASSERT2(x, w) \
    if(SYLAR_UNLIKELY(!(x))) { \
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "ASSERTION: " #x \
            << "\n" << w \
            << "\nbacktrace:\n"  \
            << sylar::BacktraceToString(100, 2, "    "); \
        assert(x); \
    } 
#endif