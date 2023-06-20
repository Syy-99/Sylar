#include "config.h"
#include "util.h"
#include "log.h"
#include "env.h"
#include <list>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

namespace sylar {

static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

ConfigVarBase::ptr Config::LookupBase(const std::string& name) {
    RWMutexType::ReadLock lock(GetMutex());
    // 查找是否支持该key
    auto it = GetDatas().find(name);
    return it == GetDatas().end() ? nullptr : it->second; 
}

//A:
//  B: 10
//  C: str
// 获得的记录应该是"A.B",10; "A.C", "str"
static void ListAllMember(const std::string& prefix,    
                          const YAML::Node& node,
                          std::list<std::pair<std::string, const YAML::Node> >& output) {
    // 非法                        
    if(prefix.find_first_not_of("abcdefghikjlmnopqrstuvwxyz._012345678") != std::string::npos) {
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "Config invalid name: " << prefix << " : " << node;
        return;
    }

    output.push_back(std::make_pair(prefix, node));    // ??? 是不是保存的形式有问题

    if(node.IsMap()) {
        for(auto it = node.begin(); it != node.end(); ++it) {
            ListAllMember(prefix.empty() ? it->first.Scalar() : prefix + "." + it->first.Scalar(), 
                          it->second, output);
        }
    }

}

void Config::LoadFromYaml(const YAML::Node& root) {
    std::list<std::pair<std::string, const YAML::Node> > all_nodes;
    ListAllMember("", root, all_nodes);

    for (auto& i : all_nodes) {
        std::string key = i.first;
        if (key.empty()) {
            continue;
        }

        // 配置文件中的大写转换为小写
        std::transform(key.begin(), key.end(), key.begin(), ::tolower);
        
        ConfigVarBase::ptr var = LookupBase(key);

        if (var) {  // 如果找到了，则根据配置文件设置该配置项的值
            if (i.second.IsScalar()) {
                var->fromString(i.second.Scalar());
            } else {
                // key: [1, 2, 3]
                std::stringstream ss;
                ss << i.second;
                var->fromString(ss.str());  // 指定对应配置的反序列化操作
            }

        }
        // 如果每找到，说明没有约定这个配置，则什么也不做

    }
}


static std::map<std::string, uint64_t> s_file2modifytime;   // 记录时间
static sylar::Mutex s_mutex;

void Config::LoadFromConfDir(const std::string& path, bool force) {
    // 获取绝对路径
    std::string absoulte_path = sylar::EnvMgr::GetInstance()->getAbsolutePath(path);
    
    // 获得这个路径下所有yaml文件
    std::vector<std::string> files;
    FSUtil::ListAllFile(files, absoulte_path, ".yml");

    for(auto& i : files) {
        {
            // 判断：只有这个yaml文件发生变化，才会加载这个yaml状态
            // 利用文件的修改时间判断 
            // 优化： 使用MD5 -> 基于文件内容
            struct stat st;
            lstat(i.c_str(), &st);
            sylar::Mutex::Lock lock(s_mutex);
            if (!force && s_file2modifytime[i] == (uint64_t)st.st_mtime) {
                // 修改时间没变
                continue;
            }
            s_file2modifytime[i] = st.st_mtime;     // 记录这个文件的修改时间
        }
        try {
            YAML::Node root = YAML::LoadFile(i);        // 加载yaml文件
            LoadFromYaml(root);     // 加载配置
            SYLAR_LOG_INFO(g_logger) << "LoadConfFile file="
                << i << " ok";
        } catch (...) {
            SYLAR_LOG_ERROR(g_logger) << "LoadConfFile file="
                << i << " failed";
        }
    }

}
void Config::Visit(std::function<void(ConfigVarBase::ptr)> cb) {
    RWMutexType::ReadLock lock(GetMutex());
    ConfigVarMap& m = GetDatas();
    for(auto it = m.begin(); it != m.end(); ++it) {
        cb(it->second);     // 定义输出的格式
    }

}



}