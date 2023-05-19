#include "config.h"
#include <list>

namespace sylar {

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

void Config::Visit(std::function<void(ConfigVarBase::ptr)> cb) {
    RWMutexType::ReadLock lock(GetMutex());
    ConfigVarMap& m = GetDatas();
    for(auto it = m.begin(); it != m.end(); ++it) {
        cb(it->second);     // 定义输出的格式
    }

}



}