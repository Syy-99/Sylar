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
     * @brief 配置参数模板子类,保存对应类型的参数值 -> 具体配置项
     */
    template<class T>
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
                return boost::lexical_cast<std::string>(m_val);
            } catch (std::exception& e) {
                SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "ConfigVar::toString exception"
                                                   << e.what() 
                                                   << " convert: " << typeid(m_val).name() << "to string";
            }
            return "";
        }

        bool fromString(const std::string& val) override {
            try {
                m_val = boost::lexical_cast<T>(val);
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

            // 假设配置参数名只允许有abcdefghikjlmnopqrstuvwxyz._012345678
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
    private:
        static ConfigVarMap s_datas;    
    };
}
#endif