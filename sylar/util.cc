#include "util.h"
#include <execinfo.h>
#include <sys/time.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
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


static int __lstat(const char* file, struct stat* st = nullptr) {
    struct stat lst;
    int ret = lstat(file, &lst);    // lstat：获取文件信息， 成功返回0
    if(st) {
        *st = lst;
    }
    return ret;
}

static int __mkdir(const char* dirname) {
    if(access(dirname, F_OK) == 0) {    //判断指定的文件或目录是否存在(F_OK)
        return 0;
    }
    return mkdir(dirname, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);   // 创建目录，并且权限775
}

bool FSUtil::Mkdir(const std::string& dirname) {
    if(__lstat(dirname.c_str()) == 0) {
        // 如果=0，则说明已经文件存在
        return true;
    }

    char* path = strdup(dirname.c_str());
    char* ptr = strchr(path + 1, '/');  // 查找字符串中某个字符第一次出现的位置， path+1是为了去除/目录
    do {
        // 可能需要递归创建目录
        for(; ptr; *ptr = '/', ptr = strchr(ptr + 1, '/')) {
            *ptr = '\0';
            if(__mkdir(path) != 0) {        // 递归创建目录
                break;
            }
        }
        if(ptr != nullptr) {
            break;
        } else if(__mkdir(path) != 0) {
            break;
        }
        free(path);
        return true;    // 递归创建成功
    } while(0);

    free(path);
    return false;   // 递归创建失败
}

bool FSUtil::IsRunningPidfile(const std::string& pidfile){
    // 获得文件状态
    if(__lstat(pidfile.c_str()) != 0) {
        // 说明没有这个文件
        return false;
    }

    // 有这个文件，则其中应该记录了服务器进程的pid
    std::ifstream ifs(pidfile);
    std::string line;
    // 读取第一行
    if(!ifs || !std::getline(ifs, line)) {
        return false;
    }
    if(line.empty()) {
        return false;
    }

    pid_t pid = atoi(line.c_str());
    if(pid <= 1) {
        return false;
    }

    if(kill(pid, 0) != 0) {  // 0: 有任何信号送出，但是系统会执行错误检查，通常会利用sig值为零来检验某个进程是否仍在执行。
        return false;
    }

    return true;    // 有文件且文件内记录的pid在运行




}
}