/**
 * @file config.h
 * @brief 配置模块
  */

#ifndef __SYLAR_CONFIG_H__
#define __SYLAR_CONFIG_H__

#include <memory>
#include <string>
#include <sstream>
#include <boost/lexical_cast.hpp>
#include <yaml-cpp/yaml.h>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <functional>

#include <iostream>

#include "log.h"
#include "thread.h"
namespace sylar {

    /**
    * @brief 配置变量的基类
    */
    class ConfigVarBase {
    public:
        typedef std::shared_ptr<ConfigVarBase> ptr;
        
        ConfigVarBase(const std::string& name, const std::string& description = "") 
            : m_name(name),
              m_description(description) {
                // key的大写转小写
                std::transform(m_name.begin(), m_name.end(), m_name.begin(), ::tolower);
            } 

        virtual ~ConfigVarBase() {}

        const std::string& getName() const { return m_name; }
        const std::string& getDescription() const { return m_description; }

        virtual std::string toString() = 0;
        virtual bool fromString(const std::string& val) = 0;

        virtual std::string getTypeName() const = 0;
    protected:
        std::string m_name;         // 配置参数的名称
        std::string m_description;  // 配置参数的描述 
    };

    /**
    * @brief 类型转换模板类(F 源类型, T 目标类型)
    */
    template<class F, class T>
    class LexicalCast{
    public:
        /**
        * @brief 类型转换
        * @param[in] v 源类型值
        * @return 返回v转换后的目标类型
        * @exception 当类型不可转换时抛出异常
        */
        T operator()(const F& v) {
            return boost::lexical_cast<T>(v);
        }
    };  

    /// 类型转换模板类偏特化(YAML String 转换成 std::vector<T>)
    template<class T>
    class LexicalCast<std::string, std::vector<T> > {
    public:
        std::vector<T> operator()(const std::string& v) {
            /*
            key:
               - 1
               - 2
            or
            key: [10,20]

            只有这种类型的yaml可以转换为vector<T>类型，否则会抛出异常，因此调用该函数的函数需要catch该异常
            */
            YAML::Node node = YAML::Load(v);

            typename std::vector<T> vec;
            std::stringstream ss;
            for(size_t i = 0; i < node.size(); ++i) {
                ss.str("");
                ss << node[i];
                vec.push_back(LexicalCast<std::string, T>()(ss.str()));
            } 
            return vec;
        }
    };

    /**
    * @brief 类型转换模板类片特化(std::vector<T> 转换成 YAML String)
    */
    template<class T>
    class LexicalCast<std::vector<T>, std::string> {
    public:
        std::string operator()(const std::vector<T>& v) {
            YAML::Node node(YAML::NodeType::Sequence);
            for(auto& i : v) {
                node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

    /// 类型转换模板类偏特化(YAML String 转换成 std::list<T>)
    template<class T>
    class LexicalCast<std::string, std::list<T> > {
    public:
        std::list<T> operator()(const std::string& v) {
            /*
            key:
               - 1
               - 2
            or
            key: [10,20]

            只有这种类型的yaml可以转换为list<T>类型，否则会抛出异常，因此调用该函数的函数需要catch该异常
            */
            YAML::Node node = YAML::Load(v);

            typename std::list<T> vec;
            std::stringstream ss;
            for(size_t i = 0; i < node.size(); ++i) {
                ss.str("");
                ss << node[i];
                vec.push_back(LexicalCast<std::string, T>()(ss.str()));
            } 
            return vec;
        }
    };

    /**
    * @brief 类型转换模板类片特化(std::list<T> 转换成 YAML String)
    */
    template<class T>
    class LexicalCast<std::list<T>, std::string> {
    public:
        std::string operator()(const std::list<T>& v) {
            YAML::Node node(YAML::NodeType::Sequence);
            for(auto& i : v) {
                node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

    /// 类型转换模板类偏特化(YAML String 转换成 std::set<T>)
    template<class T>
    class LexicalCast<std::string, std::set<T> > {
    public:
        std::set<T> operator()(const std::string& v) {
            /*
            key:
               - 1
               - 2
            or
            key: [10,20]

            只有这种类型的yaml可以转换为set<T>类型，否则会抛出异常，因此调用该函数的函数需要catch该异常
            */
            YAML::Node node = YAML::Load(v);

            typename std::set<T> vec;
            std::stringstream ss;
            for(size_t i = 0; i < node.size(); ++i) {
                ss.str("");
                ss << node[i];
                vec.insert(LexicalCast<std::string, T>()(ss.str()));
            } 
            return vec;
        }
    };

    /**
    * @brief 类型转换模板类片特化(std::set<T> 转换成 YAML String)
    */
    template<class T>
    class LexicalCast<std::set<T>, std::string> {
    public:
        std::string operator()(const std::set<T>& v) {
            YAML::Node node(YAML::NodeType::Sequence);
            for(auto& i : v) {
                node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };


    /**
    * @brief 类型转换模板类片特化(YAML String 转换成 std::unordered_set<T>)
    */
    template<class T>
    class LexicalCast<std::string, std::unordered_set<T> > {
    public:
        std::unordered_set<T> operator()(const std::string& v) {
            YAML::Node node = YAML::Load(v);
            typename std::unordered_set<T> vec;
            std::stringstream ss;
            for(size_t i = 0; i < node.size(); ++i) {
                ss.str("");
                ss << node[i];
                vec.insert(LexicalCast<std::string, T>()(ss.str()));
            }
            return vec;
        }
    };

    /**
    * @brief 类型转换模板类片特化(std::unordered_set<T> 转换成 YAML String)
    */
    template<class T>
    class LexicalCast<std::unordered_set<T>, std::string> {
    public:
        std::string operator()(const std::unordered_set<T>& v) {
            YAML::Node node(YAML::NodeType::Sequence);
            for(auto& i : v) {
                node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };


/**
 * @brief 类型转换模板类片特化(YAML String 转换成 std::map<std::string, T>)
 */
template<class T>
class LexicalCast<std::string, std::map<std::string, T> > {
public:
    std::map<std::string, T> operator()(const std::string& v) {
        /*
        key:
            key1: 1
            key2: 2
        */
        YAML::Node node = YAML::Load(v);
        typename std::map<std::string, T> vec;
        std::stringstream ss;
        for(auto it = node.begin();
                it != node.end(); ++it) {
            ss.str("");
            ss << it->second;
            vec.insert(std::make_pair(it->first.Scalar(),
                        LexicalCast<std::string, T>()(ss.str())));
        }
        return vec;
    }
};

/**
* @brief 类型转换模板类片特化(std::map<std::string, T> 转换成 YAML String)
*/
template<class T>
class LexicalCast<std::map<std::string, T>, std::string> {
public:
    std::string operator()(const std::map<std::string, T>& v) {
        YAML::Node node(YAML::NodeType::Map);
        for(auto& i : v) {
            node[i.first] = YAML::Load(LexicalCast<T, std::string>()(i.second));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

/**
* @brief 类型转换模板类片特化(YAML String 转换成 std::unordered_map<std::string, T>)
*/
template<class T>
class LexicalCast<std::string, std::unordered_map<std::string, T> > {
public:
    std::unordered_map<std::string, T> operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        typename std::unordered_map<std::string, T> vec;
        std::stringstream ss;
        for(auto it = node.begin();
                it != node.end(); ++it) {
            ss.str("");
            ss << it->second;
            vec.insert(std::make_pair(it->first.Scalar(),
                        LexicalCast<std::string, T>()(ss.str())));
        }
        return vec;
    }
};

/**
* @brief 类型转换模板类片特化(std::unordered_map<std::string, T> 转换成 YAML String)
*/
template<class T>
class LexicalCast<std::unordered_map<std::string, T>, std::string> {
public:
    std::string operator()(const std::unordered_map<std::string, T>& v) {
        YAML::Node node(YAML::NodeType::Map);
        for(auto& i : v) {
            node[i.first] = YAML::Load(LexicalCast<T, std::string>()(i.second));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};



/**
* @brief 配置参数模板子类,保存对应类型的参数值，表示一个配置项
* @details T 参数的具体类型
*          FromStr 从std::string转换成T类型的仿函数 -> 序列化 
*          ToStr 从T转换成std::string的仿函数   -> 反序列化
*          std::string 为YAML格式的字符串
*/
template<class T, class FromStr = LexicalCast<std::string, T>,
                    class ToStr = LexicalCast<T, std::string> >
class ConfigVar : public ConfigVarBase {
public:
    typedef RWMutex RWMutexType;        // 使用读写锁
    typedef std::shared_ptr<ConfigVar> ptr;
        // 回调函数
    typedef std::function<void (const T& old_value, const T& new_value)> on_change_cb;

    ConfigVar(const std::string& name, 
                const T& default_value, 
                const std::string description = " ")
            : ConfigVarBase(name, description),
                m_val(default_value) {

            }

    // 将参数值转换成YAML String
    std::string toString() override {
        try {
            RWMutexType::ReadLock lock(m_mutex);
            return ToStr()(m_val);
        } catch (std::exception& e) {
            SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "ConfigVar::toString exception"
                                                << e.what() 
                                                << " convert: " << typeid(m_val).name() << "to string";
        }
        return "";
    }
    // Yaml参数变为加载为指定的类型
    bool fromString(const std::string& val) override {
        try {
            setValue(FromStr()(val));   
            return true;
        } catch (std::exception& e) {
                SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "ConfigVar::fromString exception"
                                                << e.what() 
                                                << " convert: string to" << typeid(m_val).name();
        }
        return false;
    }
    
    const T getValue() { 
        RWMutexType::ReadLock lock(m_mutex);
        return m_val; 
    }
    void setValue(const T& v) {
        {
            RWMutexType::ReadLock lock(m_mutex); 
            if (v == m_val) {
                return;
            }

            // 执行回调函数
            for (auto& i : m_cbs) {
                i.second(m_val, v);
            }
            // 读锁在这里释放
        }
        RWMutexType::WriteLock lock(m_mutex);
        m_val = v;
        
    }

    std::string getTypeName() const override { return typeid(T).name(); }

    /**
    * @brief 注册回调函数
    * @return 返回该回调函数对应的唯一id,用于删除回调
    */
    uint64_t addListener(on_change_cb cb) {
        static uint64_t s_fun_id = 0;       // 内部确定id
        RWMutexType::WriteLock lock(m_mutex);
        ++s_fun_id;
        m_cbs[s_fun_id] = cb;
        return s_fun_id;
    }

    void delListener(uint64_t key) {
        RWMutexType::WriteLock lock(m_mutex);
        m_cbs.erase(key);
    }

    on_change_cb getListener(uint64_t key) {
            RWMutexType::ReadLock lock(m_mutex);
        auto it = m_cbs.find(key);
        return it == m_cbs.end() ? nullptr : it->second;
    }

    void clearListener() {
        RWMutexType::WriteLock lock(m_mutex);
        m_cbs.clear();
    }
    
private:
    RWMutexType m_mutex;
    T m_val;    // 配置的值，有实际的类型

    // 变更回调函数组, uint64_t key,要求唯一，一般可以用hash
    std::map<uint64_t, on_change_cb> m_cbs;
    // function没有比较函数，所有必须拿一个哈希key来确定某一个函数
};

/// ConfigVar的管理类，管理所有的配置项
class Config {
public:
    typedef std::unordered_map<std::string, ConfigVarBase::ptr> ConfigVarMap;
    typedef RWMutex RWMutexType;

    //  获取/创建对应参数名的配置参数
    template<class T>
    static typename ConfigVar<T>::ptr Lookup(const std::string& name, 
                                                const T& default_value, 
                                                const std::string& description = "") {
        RWMutexType::WriteLock lock(GetMutex());
        auto it = GetDatas().find(name);   
        if (it != GetDatas().end()) {
            auto tmp = std::dynamic_pointer_cast<ConfigVar<T> >(it->second);   
            if (tmp) {
                SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "Lookup name=" << name << " exists";
                return tmp;
            } else {
                // 找到了，但是类型不符合，导致类型转换操作返回空指针
                SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "Lookup name=" << name << " exists but type not "
                    << typeid(T).name() << " real_type=" << it->second->getTypeName() << " " << it->second->toString();
                return nullptr;
            }
        }                     

        // 假设配置参数名中只允许有abcdefghikjlmnopqrstuvwxyz._012345678
        if (name.find_first_not_of("abcdefghikjlmnopqrstuvwxyz._012345678") != std::string::npos) {
            SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "Lookup name invalid " << name;
            throw std::invalid_argument(name);
        }
 
        // 创建这个配置
        typename ConfigVar<T>::ptr v(new ConfigVar<T>(name, default_value, description));
        GetDatas()[name] = v;
        return v;

    }

    // 查找配置参数
    template<class T>
    static typename ConfigVar<T>::ptr Lookup(const std::string& name) {
        RWMutexType::ReadLock lock(GetMutex());
        auto it = GetDatas().find(name);
        if (it == GetDatas().end())
            return nullptr;
        return std::dynamic_pointer_cast<ConfigVar<T> >(it->second);
    }

    /// 使用YAML::Node初始化配置模块
    static void LoadFromYaml(const YAML::Node& root);

    /**
     * @brief 加载path文件夹里面的配置文件
     * force：是否强制加载
     */
    static void LoadFromConfDir(const std::string& path, bool force = false);


    /// 查找配置参数,返回配置参数的基类
    static ConfigVarBase::ptr LookupBase(const std::string& name);

     /**
     * @brief 遍历配置模块里面所有配置项
     * @param[in] cb 配置项回调函数 
        方便测试，打印载入后的yaml
     */
    static void Visit(std::function<void(ConfigVarBase::ptr)> cb);
private:
    static ConfigVarMap& GetDatas() {
        // 利用函数，确保静态成员变量s_datas在静态方法Lookup之前调用
        static ConfigVarMap s_datas;    
        return s_datas;
    }

    /**
    * @brief 配置项的RWMutex
    */
    static RWMutexType& GetMutex() {
        static RWMutexType s_mutex;
        return s_mutex;
    }
};
}
#endif