/*
    参数解析
*/

#ifndef __SYLAR_ENV_H__
#define __SYLAR_ENV_H__

#include "singleton.h"
#include "mutex.h"
#include <map>
#include <vector>
#include <iostream>

namespace sylar {
class Env {
public:
    typedef RWMutex RWMutexType;

    // 解析外面传递的参数
    bool init(int argc, char** argv);

    void add(const std::string& key, const std::string& val);
    bool has(const std::string& key);
    void del(const std::string& key);
    std::string get(const std::string& key, const std::string& default_value = "");

    void addHelp(const std::string& key, const std::string& desc);  //增加help信息
    void removeHelp(const std::string& key);    // 删除help信息
    void printHelp();   // 打印help信息
private:
    RWMutexType m_mutex; 

    //key: 参数， value: 参数值
    std::map<std::string, std::string> m_args;

    // pair: 参数名称 - 参数实际意义
    std::vector<std::pair<std::string, std::string> > m_helps;

    // // 上下文相关参数
    std::string m_program;  // 表示当前应用名称
    // std::string m_exe;
    // std::string m_cwd;
};

typedef sylar::Singleton<Env> EnvMgr;

}

#endif