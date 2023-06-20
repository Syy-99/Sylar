#include "env.h"
#include "log.h"
#include <unistd.h>
#include <iostream>
#include <iomanip>
#include <stdlib.h>
namespace sylar {

static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");


bool Env::init(int argc, char** argv) {
    char link[1024] = {0};
    char path[1024] = {0};
    sprintf(link, "/proc/%d/exe", getpid());
    readlink(link, path, sizeof(path));     // readlink - print resolved symbolic links or canonical file names
    m_exe = path;

    auto pos = m_exe.find_last_of("/");     // 倒着找第一个'/'
    m_cwd = m_exe.substr(0, pos) + "/";
    
    SYLAR_LOG_INFO(g_logger) << "m_cwd: " << m_cwd;
    // 外面传参的的形式 -config /path/to/config -file xxxx -d
    m_program = argv[0];
    const char* now_key = nullptr;
    for(int i = 1; i < argc; ++i) {
        if(argv[i][0] == '-') {     // key值必须以-开头
            if(strlen(argv[i]) > 1) {
                if (now_key) {      // 说明上一个参数key是没有参数的
                    add(now_key, "");
                }
                now_key = argv[i] + 1;
            } else {
                SYLAR_LOG_ERROR(g_logger) << "invalid arg idx=" << i
                    << " val=" << argv[i];
                return false;
            }
        } else {
            if(now_key) {
                add(now_key, argv[i]);
                now_key = nullptr;      // 清除这个key
            } else {
                SYLAR_LOG_ERROR(g_logger) << "invalid arg idx=" << i
                    << " val=" << argv[i];
                return false;
            }
        }
    }
    if(now_key) {
        add(now_key, "");       // 支持key是空值
    }
    return true;
}

void Env::add(const std::string& key, const std::string& val) {
    RWMutexType::WriteLock lock(m_mutex);
    m_args[key] = val;
}

bool Env::has(const std::string& key) {
    RWMutexType::ReadLock lock(m_mutex);
    auto it = m_args.find(key);
    return it != m_args.end();
}

void Env::del(const std::string& key) {
    RWMutexType::WriteLock lock(m_mutex);
    m_args.erase(key);
}

std::string Env::get(const std::string& key, const std::string& default_value) {
    RWMutexType::ReadLock lock(m_mutex);
    auto it = m_args.find(key);
    return it != m_args.end() ? it->second : default_value;
}

void Env::addHelp(const std::string& key, const std::string& desc) {
    removeHelp(key);    // 先删除
    RWMutexType::WriteLock lock(m_mutex);
    m_helps.push_back(std::make_pair(key, desc));   // 再添加
}

void Env::removeHelp(const std::string& key) {
    RWMutexType::WriteLock lock(m_mutex);
    for(auto it = m_helps.begin();
            it != m_helps.end();) {
        if(it->first == key) {
            it = m_helps.erase(it);
        } else {
            ++it;
        }
    }
}

void Env::printHelp() {
    RWMutexType::ReadLock lock(m_mutex);
    std::cout << "Usage: " << m_program << " [options]" << std::endl;
    for(auto& i : m_helps) {
        std::cout << std::setw(5) << "-" << i.first << " : " << i.second << std::endl;
    }
}

bool Env::setEnv(const std::string& key, const std::string& val) {
    return !setenv(key.c_str(), val.c_str(), 1);        // 1：允许覆盖
    /// 只会改变自己的环境变量
}

std::string Env::getEnv(const std::string& key, const std::string& default_value) {
    const char* v = getenv(key.c_str());    //  get an environment variable
    //  The strings in the environment list are of the form name=value.
    if(v == nullptr) {
        return default_value;
    }
    return v;
}

std::string Env::getAbsolutePath(const std::string& path) const {
    if(path.empty()) {  // 如果是空，则返回根路径
        return "/";
    }
    if(path[0] == '/') {    // 如果path本身就是绝对路径
        return path;
    }
    // 否则，path就是一个相对路径，则直接拼接
    return m_cwd + path;
}



}