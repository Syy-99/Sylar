#include "util.h"
#include <execinfo.h>
#include <sys/time.h>
#include <dirent.h>
#include <unistd.h>

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

std::string Time2Str(time_t ts, const std::string& format) {
    struct tm tm;
    localtime_r(&ts, &tm);
    char buf[64];
    strftime(buf, sizeof(buf), format.c_str(), &tm);
    return buf;
}

time_t Str2Time(const char* str, const char* format) {
    struct tm t;
    memset(&t, 0, sizeof(t));
    if(!strptime(str, format, &t)) {
        return 0;
    }
    return mktime(&t);
}


void FSUtil::ListAllFile(std::vector<std::string>& files
                            ,const std::string& path
                            ,const std::string& subfix) {
    // 尝试访问这个路径，以确保这个路径存在
    if(access(path.c_str(), 0) != 0) {
        return;
    }
    
    // 打开这个路径
    DIR* dir = opendir(path.c_str());
    if(dir == nullptr) {
        return;
    }

    // 读这个路径下的文件
    struct dirent* dp = nullptr;
    while((dp = readdir(dir)) != nullptr) {
        if(dp->d_type == DT_DIR) {  // 如果是文件夹
            if(!strcmp(dp->d_name, ".")
                || !strcmp(dp->d_name, "..")) {
                continue;
            }
            // 继续往下读
            ListAllFile(files, path + "/" + dp->d_name, subfix);
        } else if (dp->d_type == DT_REG) {  // 如果是普通文件
            std::string filename(dp->d_name);   // 获得这个文件的名字
            if(subfix.empty()) {
                files.push_back(path + "/" + filename);     // 构造这个文件的绝对路径
            } else {
                // 检查后缀是否满足
                if(filename.size() < subfix.size()) {
                    continue;
                }
                if(filename.substr(filename.length() - subfix.size()) == subfix) {
                    files.push_back(path + "/" + filename);
                }
            }
        }
    }
    
    // 读完了需要关闭这个路径
    closedir(dir);                             
}

}