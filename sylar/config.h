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
#include "log.h"

#include <iostream>

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
    protected:
        std::string m_name; // 配置参数的名称
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
    
    /**
    * @brief 配置参数模板子类,保存对应类型的参数值
    * @details T 参数的具体类型
    *          FromStr 从std::string转换成T类型的仿函数 -> 序列化 
    *          ToStr 从T转换成std::string的仿函数   -> 反序列化
    *          std::string 为YAML格式的字符串
    */
    template<class T, class FromStr = LexicalCast<std::string, T>,
                      class ToStr = LexicalCast<T, std::string> >
    class ConfigVar : public ConfigVarBase {
    public:
        typedef std::shared_ptr<ConfigVar> ptr;

        ConfigVar(const std::string& name, 
                  const T& default_value, 
                  const std::string description = " ")
                : ConfigVarBase(name, description),
                  m_val(default_value) {

                }

        // 将参数值转换成YAML String
        std::string toString() override {
            try {
                 return ToStr()(m_val);
            } catch (std::exception& e) {
                SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "ConfigVar::toString exception"
                                                   << e.what() 
                                                   << " convert: " << typeid(m_val).name() << "to string";
            }
            return "";
        }

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
        
        const T getValue() const { return m_val; }
        void setValue(const T& v) { m_val = v; }
    private:
        T m_val;
    };

    /// ConfigVar的管理类
    class Config {
    public:
        typedef std::map<std::string, ConfigVarBase::ptr> ConfigVarMap;


        //  获取/创建对应参数名的配置参数
        template<class T>
        static typename ConfigVar<T>::ptr Lookup(const std::string& name, 
                                                 const T& default_value, 
                                                 const std::string& description = "") {
            auto tmp = Lookup<T>(name);
            if (tmp) {
                SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "Lookup name=" << name << " exists";
                return tmp;
            }

            // 假设配置参数名中只允许有abcdefghikjlmnopqrstuvwxyz._012345678
            if (name.find_first_not_of("abcdefghikjlmnopqrstuvwxyz._012345678") != std::string::npos) {
                SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "Lookup name invalid " << name;
                throw std::invalid_argument(name);
            }

            // 创建这个配置
            typename ConfigVar<T>::ptr v(new ConfigVar<T>(name, default_value, description));
            s_datas[name] = v;
            return v;

        }

        // 查找配置参数
        template<class T>
        static typename ConfigVar<T>::ptr Lookup(const std::string& name) {
            auto it = s_datas.find(name);
            if (it == s_datas.end())
                return nullptr;
            return std::dynamic_pointer_cast<ConfigVar<T> >(it->second);
        }

        /// 使用YAML::Node初始化配置模块
        static void LoadFromYaml(const YAML::Node& root);

        /// 查找配置参数,返回配置参数的基类
        static ConfigVarBase::ptr LookupBase(const std::string& name);
    private:
        static ConfigVarMap s_datas;    
    };
}
#endif