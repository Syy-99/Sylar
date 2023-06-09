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
#include <vector>
#include <string>
#include <cstring>

namespace sylar {

// 获得线程id
pid_t GetThreadId();
// 获得协程id
uint32_t GetFiberId();


/**
 * @brief 获取当前的调用栈
 * @param[out] bt 保存调用栈
 * @param[in] size 最多返回层数
 * @param[in] skip 跳过栈顶的层数
 */
void Backtrace(std::vector<std::string>& bt, int size = 64, int skip = 1);

/**
 * @brief 获取当前栈信息的字符串
 * @param[in] size 栈的最大层数
 * @param[in] skip 跳过栈顶的层数
 * @param[in] prefix 栈信息前输出的内容
 */
std::string BacktraceToString(int size = 64, int skip = 2, const std::string& prefix = "");


// 时间ms
uint64_t GetCurrentMS();
uint64_t GetCurrentUS();


std::string Time2Str(time_t ts = time(0), const std::string& format = "%Y-%m-%d %H:%M:%S");
time_t Str2Time(const char* str, const char* format = "%Y-%m-%d %H:%M:%S");


// 文件的相关操作
class FSUtil {
public:
    // 列出path下的文件名
    // subfix: 指定特定后缀
    static void ListAllFile(std::vector<std::string>& files
                            ,const std::string& path
                            ,const std::string& subfix);
    // 启动时创建pid文件，记录该进程的pid
    // dirname: pid文件的绝对路径
    static bool Mkdir(const std::string& dirname);
    // 判断是否有pid文件，如果有，则认为服务已经在运行
    // pidfile: pid文件的绝对路径
    static bool IsRunningPidfile(const std::string& pidfile);

};

}



#endif