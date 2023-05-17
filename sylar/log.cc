#include "log.h"
#include <iostream>
#include <map>
#include <functional>
#include <time.h>
#include "config.h"

namespace sylar {

    const char* LogLevel::ToString(LogLevel::Level level) {
        switch(level) {
    #define XX(name) \
        case LogLevel::name: \
            return #name; \
            break;

        XX(DEBUG);
        XX(INFO);
        XX(WARN);
        XX(ERROR);
        XX(FATAL);
    #undef XX
        default:
            return "UNKNOW";
        }
        return "UNKNOW";
    }

    LogLevel::Level LogLevel::FromString(const std::string& str) {
#define XX(level, v) \
    if(str == #v) { \
        return LogLevel::level; \
    }
    XX(DEBUG, debug);
    XX(INFO, info);
    XX(WARN, warn);
    XX(ERROR, error);
    XX(FATAL, fatal);

    XX(DEBUG, DEBUG);
    XX(INFO, INFO);
    XX(WARN, WARN);
    XX(ERROR, ERROR);
    XX(FATAL, FATAL);
    return LogLevel::UNKNOW;
#undef XX
    }
    LogEventWrap::LogEventWrap(LogEvent::ptr e) : m_event(e) {

    }
    LogEventWrap::~LogEventWrap() {
        m_event->getLogger()->log(m_event->getLevel(), m_event);
    }
    std::stringstream& LogEventWrap::getSS() {
        return m_event->getSS();
    }

    void LogEvent::format(const char* fmt, ...) {
        va_list al;
        va_start(al, fmt);
        format(fmt, al);
        va_end(al);
    }

    void LogEvent::format(const char* fmt, va_list al) {
        char* buf = nullptr;
        int len = vasprintf(&buf, fmt, al); // 将格式化数据从可变参数列表写入缓冲区
        if (len != -1) {
            m_ss<< std::string(buf, len);
            free(buf);
        }
    }
    
    /*
     * 定义几种格式控制符
     * %m 消息体
     * %p level
     * %r 启动耗时
     * %c 日志名称
     * %t 线程id
     * %n 回车换行
     * %d 时间
     * %f 文件名
     * %l 行号
     */
    class MessageFormatItem : public LogFormatter::FormatItem {
    public:
        MessageFormatItem(const std::string& str) {}
        void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
            os<<event->getContent();
        }
    };
    class LevelFormatItem : public LogFormatter::FormatItem {
    public:
        LevelFormatItem(const std::string& str) {}
        void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
            os<<LogLevel::ToString(level);
        }
    };
    class ElapseFormatItem : public LogFormatter::FormatItem {
    public:
        ElapseFormatItem(const std::string& str) {}
        void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
            os<<event->getElapse();
        }
    };
    class NameFormatItem : public LogFormatter::FormatItem {
    public:
        NameFormatItem(const std::string& str) {}
        void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
            // os<<logger->getName();
              os<<event->getLogger()->getName();    // 返回最原始的Logger名称，而不是实际调用的Logger名称
        }
    };
    class ThreadIdFormatItem : public LogFormatter::FormatItem {
    public:
        ThreadIdFormatItem(const std::string& str) {}
        void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
            os<<event->getThreadId();
        }
    };
    class DateTimeFormatItem : public LogFormatter::FormatItem {
    public:
        DateTimeFormatItem(const std::string& format = "%Y-%m-%d %H:%M:%S")
        :m_format(format) {
            if(m_format.empty()) {
                m_format = "%Y-%m-%d %H:%M:%S";
            }
         }

        void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
            struct tm tm;
            time_t time = event->getTime();
            localtime_r(&time, &tm);
            char buf[64];
            
            strftime(buf, sizeof(buf), m_format.c_str(), &tm);
            // std::cout<<"data level: "<<level<<" "<<buf<<"?"<<time<<"?"<<m_format.c_str()<<std::endl;
            os<<buf;
        }
    private:
        std::string  m_format;
    };
    class FilenameFormatItem : public LogFormatter::FormatItem {
    public:
        FilenameFormatItem(const std::string& str) {}
        void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
            os<<event->getFile();
        }
    };

    class LineFormatItem : public LogFormatter::FormatItem {
    public:
        LineFormatItem(const std::string& str) {}
        void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
            os<<event->getLine();
        }
    };

    class NewLineFormatItem : public LogFormatter::FormatItem {
    public:
        NewLineFormatItem(const std::string& str) {}
        void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
            os<<std::endl;
        }
    };

    class StringFormatItem : public LogFormatter::FormatItem {
    public:
       StringFormatItem(const std::string& str) : m_string(str) {}
        void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
            os<<m_string;
        }
    private:
        std::string m_string;
    };

    class TabFormatItem : public LogFormatter::FormatItem {
    public:
       TabFormatItem(const std::string& str) {}
        void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
            os<<'\t';
        }
    private:
    };

    class FiberIdFormatItem : public LogFormatter::FormatItem {
    public:
       FiberIdFormatItem(const std::string& str) {}
        void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
            os<<event->getFiberId();
        }
    private:
    };

    Logger::Logger(const std::string &name) : m_name(name), m_level(LogLevel::DEBUG) {
        m_formatter.reset(new LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"));

    }

    void Logger::setFormatter(LogFormatter::ptr val) {
        MutexType::Lock lock(m_mutex);
        m_formatter = val;
        
        for(auto&i : m_appenders) {
            MutexType::Lock lock(i->m_mutex);
            if (!i->m_hasFormatter) {
                i->m_formatter = m_formatter;       // Q: 为什们不用appender的setFormatter呢？ 因为setFormatter会改变m_hasFormatter
                                                    // 但是此时的formatter并不是自己的，所以不能直接调
            }
        }
    }

    void Logger::setFormatter(const std::string& val) {
        sylar::LogFormatter::ptr new_val(new sylar::LogFormatter(val));
        if (new_val->isError()) {
            std::cout << "Logger setFormatter name=" << m_name
            << " value=" << val << " invalid formatter"
            << std::endl;
            return;
        } 
        setFormatter(new_val);  
    }

    LogFormatter::ptr Logger::getFormatter() {
        MutexType::Lock lock(m_mutex);
        return m_formatter;
    }

    void Logger::log(LogLevel::Level level, LogEvent::ptr event) {
        if (level >= m_level) {
            auto self = shared_from_this();   // 返回一个当前类的std::share_ptr
            MutexType::Lock lock(m_mutex);
            if (!m_appenders.empty()) {
                // 遍历每个目的地，根据日志级别来判断该日志是否可以输出到该目的地
                for (auto &it: m_appenders) {
                    it->log(self, level, event);
                }
            } else if (m_root){ 
                // 如果某个用户自定义的Logger没有指定appender,设置为无效，则仍然通过root日志器写
                m_root->log(level, event);
            }
        }
    }

    LogEvent::LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level,const char* file,
                        int32_t line, uint32_t elapse, uint32_t thread_id, uint32_t fiber_id, uint64_t time) 
    : m_file(file), m_line(line), m_elapse(elapse), m_threadId(thread_id), 
      m_fiberId(fiber_id), m_time(time), m_logger(logger), m_level(level) {

    }

    void Logger::debug(LogEvent::ptr event) {
        log(LogLevel::DEBUG, event);
    }

    void Logger::info(LogEvent::ptr event) {
        log(LogLevel::INFO, event);
    }

    void Logger::warn(LogEvent::ptr event) {
        log(LogLevel::WARN, event);
    }

    void Logger::error(LogEvent::ptr event) {
        log(LogLevel::ERROR, event);
    }

    void Logger::fatal(LogEvent::ptr event) {
        log(LogLevel::FATAL, event);
    }



    void Logger::addAppender(LogAppender::ptr appender) {
        MutexType::Lock lock(m_mutex);      // 访问Logger的appender需要加锁
        if (!appender->getFormatter()) {
            MutexType::Lock lock(appender->m_mutex);  // 修改这个appender内部需要加锁
            appender->m_formatter = m_formatter;
            // appender->setFormatter(m_formatter);    // 如果appender没有指定，则继承logger日志器的
        }
        m_appenders.push_back(appender);
    }

    void Logger::delAppender(LogAppender::ptr appender) {
        MutexType::Lock lock(m_mutex);
        for (auto it = m_appenders.begin(); it != m_appenders.end(); ++it) {
            if (*it == appender) {
                m_appenders.erase(it);
                break;
            }
        }
    }

    void Logger::clearAppenders() {
        MutexType::Lock lock(m_mutex);
        m_appenders.clear();
    }

    std::string Logger::toYamlString() {
        MutexType::Lock lock(m_mutex);
        YAML::Node node;
        node["name"] = m_name;
        if(m_level != LogLevel::UNKNOW) {
            node["level"] = LogLevel::ToString(m_level);
        }

        if (m_formatter) {    // 访问加锁
            node["formatter"] = m_formatter->getPattern();
        }

        for(auto& i : m_appenders) {    // 访问加锁
            node["appenders"].push_back(YAML::Load(i->toYamlString()));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }

    void LogAppender::setFormatter(LogFormatter::ptr val) {
        MutexType::Lock lock(m_mutex);
        m_formatter = val;
        if (m_hasFormatter && m_formatter){
            m_hasFormatter = true;
        } else {
            m_hasFormatter = false;
        }
    }

    LogFormatter::ptr LogAppender::getFormatter() {
        // 因为m_formatter可能被其他线程改变，所以访问需要加锁
        MutexType::Lock lock(m_mutex);
        return m_formatter;
    }

    bool FileLogAppender::reopen() {
        MutexType::Lock lock(m_mutex);
        if (m_filestream) {
            // 如果已经打开了文件，则需要先关闭
            m_filestream.close();
        }
        m_filestream.open(m_filename);

        return !!m_filestream;   // 返回是否打开成功
    }

    void StdoutLogAppender::log(std::shared_ptr<Logger> logger,LogLevel::Level level, LogEvent::ptr event) {
        if (level >= m_level) {
            // 按格式构造日志信息，并输出到指定的日志输出地
            MutexType::Lock lock(m_mutex);      // std::cout输出也要加，确保有序输出
            std::cout << m_formatter->format(logger, level, event);  // 输出到标准输出流中
        }

    }
    std::string StdoutLogAppender::toYamlString() {
        MutexType::Lock lock(m_mutex);
        YAML::Node node;
        node["type"] = "StdoutLogAppender";
        if(m_level != LogLevel::UNKNOW) {
            node["level"] = LogLevel::ToString(m_level);
        }
        if(m_formatter) {
            node["formatter"] = m_formatter->getPattern();
        }

        std::stringstream ss;
        ss << node;
        return ss.str();
    }

    std::string FileLogAppender::toYamlString() {
        MutexType::Lock lock(m_mutex);
        YAML::Node node;
        node["type"] = "FileLogAppender";
        node["file"] = m_filename;
        if(m_level != LogLevel::UNKNOW) {
            node["level"] = LogLevel::ToString(m_level);
        }
        if(m_formatter) {
            node["formatter"] = m_formatter->getPattern();
        }

        std::stringstream ss;
        ss << node;
        return ss.str();
    }
    void FileLogAppender::log(std::shared_ptr<Logger> logger,LogLevel::Level level, LogEvent::ptr event) {
        if (level >= m_level) {
            // 按格式构造日志信息，并输出到指定的日志输出地
            // std::cout<<"file_appender log"<<std::endl;
            MutexType::Lock lock(m_mutex);
            m_filestream << m_formatter->format(logger, level, event);   // 输出到文件流中
        }
    }

    FileLogAppender::FileLogAppender(const std::string &filename) : m_filename(filename) {
        reopen();   // 根据参数获得文件流
    }

    LogFormatter::LogFormatter(const std::string &pattern) : m_pattern(pattern) {
        init();
    }

    std::string LogFormatter::format(std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) {
        std::stringstream ss;
        // 由格式项，输出LogFormatter的每个日志格式
        for (auto &i: m_items) {
            // std::cout<<"formatter"<<std::endl;
            i->format(ss, logger, level, event);
            // std::cout<<ss.str()<<"?"<<std::endl;
        }
        
        return ss.str();
    }


    // 主要的格式形式： %xx %xx{xxx} %%。 例如： %d %f{0.1} %%
    // 假设m_pattern = "str: %%, %f{1,1},hhh"
    // 可以尝试使用有限状态机实现？？？？
    void LogFormatter::init() {
        // std::cout<<"my_pattern: "<< m_pattern<<std::endl;
        
        // 保存每个格式控制符的内容
        // str, format, type :<f, 1.1, 1>
        std::vector<std::tuple<std::string, std::string, int> > vec;

        std::string nstr;    // 表示当前解析出的格式控制符前的普通字符，对应 str: 
        for (size_t i = 0; i < m_pattern.size(); ++i) {
            if (m_pattern[i] != '%') {
                // 非格式控制符，直接保存即可
                nstr.append(1, m_pattern[i]);
                continue;
            }


            // 每个格式控制符都是以 % 开头的
            // 开始解析%后面的字符,
            size_t n = i + 1;
            // 解析格式控制符：%%
            if (n < m_pattern.size() && m_pattern[n] == '%') {
                nstr.append(1, '%');
                continue;
            }

            // 解析格式控制符：%f{1.1}
            int fmt_status = 0; // 状态
            std::string str; // 保存f
            std::string fmt; // 保存1.1
            size_t fmt_begin = 0;
            while (n < m_pattern.size()) {
                if (!fmt_status && (!isalpha(m_pattern[n]) && m_pattern[n] != '{'
                    && m_pattern[n] != '}')) {
                    str = m_pattern.substr(i + 1, n - i - 1);
                    break;
                }
                if (fmt_status == 0) {
                    if ((m_pattern[n] == '{')) {
                        str = m_pattern.substr(i + 1, n - i - 1);
                        fmt_status = 1;
                        fmt_begin = n;      // 不写成=n+1,是为了下面调用substr的格式一致
                        ++n;
                        continue;
                    }
                }

                if (fmt_status == 1) {
                    if (m_pattern[n] == '}') {
                        fmt = m_pattern.substr(fmt_begin + 1, n - fmt_begin - 1);
                        fmt_status = 0;
                        ++n;
                        break;
                    }
                }
                ++n;

                if(n == m_pattern.size()) {
                    if(str.empty()) {
                        str = m_pattern.substr(i + 1);
                }
            }
            }

            if (fmt_status == 0) {
                // 处理 str:
                if (!nstr.empty()) {
                    vec.push_back(std::make_tuple(nstr, "", 0));
                    nstr.clear();
                }
                // str = m_pattern.substr(i + 1, n - i - 1);
                vec.push_back(std::make_tuple(str, fmt, 1));
                i = n - 1;
            } else if (fmt_status == 1) {
                std::cout << "pattern parse error: " << m_pattern << " - " << m_pattern.substr(i) << std::endl;
                vec.push_back(std::make_tuple("<<pattern error>>", fmt, 0));
                m_error = true ;
            }
        }

        // 处理 hhh
        if (!nstr.empty()) {
            vec.push_back(std::make_tuple(nstr, "", 0));
        }

        /*
         * 定义几种格式控制符
         * %m 消息体
         * %p level
         * %r 启动耗时
         * %c 日志名称
         * %t 线程id
         * %n 回车换行
         * %d 时间
         * %f 文件名
         * %l 行号
         */

        // %d->格式项类
        // std::function<FormatItem::ptr(const std::string& str)??
        static std::map<std::string, std::function<FormatItem::ptr(const std::string& str)> > s_format_iterm = {
#define XX(str,C) \
        {#str, [](const std::string &fmt){ return FormatItem::ptr(new C(fmt));} } \

        XX(m, MessageFormatItem),
        XX(p, LevelFormatItem),
        XX(r, ElapseFormatItem),
        XX(c, NameFormatItem),
        XX(t, ThreadIdFormatItem),
        XX(n, NewLineFormatItem),
        XX(d, DateTimeFormatItem),
        XX(f, FilenameFormatItem),
        XX(l, LineFormatItem),
        XX(T, TabFormatItem),
        XX(F, FiberIdFormatItem),
#undef XX
        };

        for (auto &i : vec) {
            if (std::get<2>(i) == 0) {  // 普通string
                m_items.push_back(FormatItem::ptr(new StringFormatItem(std::get<0>(i))));
            } else {
                auto it = s_format_iterm.find(std::get<0>(i));
                if (it == s_format_iterm.end()) {   // 错误
                    m_items.push_back(FormatItem::ptr(new StringFormatItem("<<error_format %" + std::get<0>(i) + ">>")));
                    // 记录错误
                    m_error = true;
                } else {
                    m_items.push_back(it->second(std::get<1>(i)));
                }
            }

        //    std::cout << "(" << std::get<0>(i) << ") - (" << std::get<1>(i) << ") - (" << std::get<2>(i) << ")" << std::endl;
        }

        // std::cout << m_items.size() << std::endl;
    }

LoggerManager::LoggerManager() {
    m_root.reset(new Logger);
    m_root->addAppender(LogAppender::ptr(new StdoutLogAppender));

    m_loggers[m_root->m_name] = m_root;
    
    init();

}

Logger::ptr LoggerManager::getLogger(const std::string& name) {
    MutexType::Lock lock(m_mutex);
    auto it = m_loggers.find(name);

    if (it != m_loggers.end()) {
        return it->second;
    }

    // 不存在
    Logger::ptr logger(new Logger(name));   // 只有名字，其他都没有
    logger->m_root = m_root;
    m_loggers[name] = logger;
    return logger;

}


struct LogAppenderDefine {
    int type = 0; //1 File, 2 Stdout
    LogLevel::Level level = LogLevel::UNKNOW;
    std::string formatter;
    std::string file;

    bool operator==(const LogAppenderDefine& oth) const {
        return type == oth.type
            && level == oth.level
            && formatter == oth.formatter
            && file == oth.file;
    }
};

/// 读取配置文件中某个配置器的一系列属性
struct LogDefine {
    std::string name;
    LogLevel::Level level = LogLevel::UNKNOW;
    std::string formatter;
    std::vector<LogAppenderDefine> appenders;

    // 因为可能存在回调机制，判断是否相等，所以必须提供重载=运算符
    bool operator==(const LogDefine& oth) const {
        return name == oth.name
            && level == oth.level
            && formatter == oth.formatter
            && appenders == appenders;
    }

    // 为后面set集合重载<运算符
    bool operator<(const LogDefine& oth) const {
        return name < oth.name;
        //Q: 创建Logger的时候，名字应该是可能重复的?
    }

    bool isValid() const {
        return !name.empty();
    }
};

// 新的类的序列化与反序列化
template<>
class LexicalCast<std::string, LogDefine> {
public:
    LogDefine operator()(const std::string& v) {
        YAML::Node n = YAML::Load(v);
        LogDefine ld;

        // 约定：每个日志器必须有名字
        if(!n["name"].IsDefined()) {    // 判断是否有这个属性
            std::cout << "log config error: name is null, " << n
                      << std::endl;
            throw std::logic_error("log config name is null");
        }
        ld.name = n["name"].as<std::string>();

        ld.level = LogLevel::FromString(n["level"].IsDefined() ? n["level"].as<std::string>() : "");

        if(n["formatter"].IsDefined()) {
            ld.formatter = n["formatter"].as<std::string>();
        }

        // appenders是一个序列
        if(n["appenders"].IsDefined()) {
            for(size_t x = 0; x < n["appenders"].size(); ++x) {
                auto a = n["appenders"][x];
                if(!a["type"].IsDefined()) {    // 没有指定输出地的类型
                    std::cout << "log config error: appender type is null, " << a
                              << std::endl;
                    continue;
                }

                std::string type = a["type"].as<std::string>();
                LogAppenderDefine lad;

                if(type == "FileLogAppender") {
                    lad.type = 1;
                    if(!a["file"].IsDefined()) {    // 没有指定日志文件
                        std::cout << "log config error: fileappender file is null, " << a
                              << std::endl;
                        continue;
                    }
                    lad.file = a["file"].as<std::string>();
                    if(a["formatter"].IsDefined()) {
                        lad.formatter = a["formatter"].as<std::string>();
                    }
                } else if(type == "StdoutLogAppender") {
                    lad.type = 2;
                    if(a["formatter"].IsDefined()) {
                        lad.formatter = a["formatter"].as<std::string>();
                    }
                } else {
                    // 出现了未定义的类型
                    std::cout << "log config error: appender type is invalid, " << a
                              << std::endl;
                    continue;
                }
                ld.appenders.push_back(lad);
            }   
        }

        return ld;
    }
};

template<>
class LexicalCast<LogDefine, std::string> {
public:
    std::string operator()(const LogDefine& i) {
        YAML::Node n;

        n["name"] = i.name;     // 确保i.name不为空

        if(i.level != LogLevel::UNKNOW) {
            n["level"] = LogLevel::ToString(i.level);
        }

        // 约定： 如果没有设置foramter, 则不写
        if(!i.formatter.empty()) { 
            n["formatter"] = i.formatter;
        }

         for(auto& a : i.appenders) {
            YAML::Node na;
            if(a.type == 1) {
                na["type"] = "FileLogAppender";
                na["file"] = a.file;
            } else if(a.type == 2) {
                na["type"] = "StdoutLogAppender";
            }

            if(a.level != LogLevel::UNKNOW) {
                na["level"] = LogLevel::ToString(a.level);
            }

            if(!a.formatter.empty()) {
                na["formatter"] = a.formatter;
            }

            n["appenders"].push_back(na);
        }
        std::stringstream ss;
        ss << n;
        return ss.str();
    }
};


sylar::ConfigVar<std::set<LogDefine> >::ptr g_log_defines =
    sylar::Config::Lookup("logs", std::set<LogDefine>(), "logs config");


struct LogIniter {
    LogIniter() {
        g_log_defines->addListener(0xF1E231, 
        [](const std::set<LogDefine>& old_value, const std::set<LogDefine>& new_value){
            SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "on_logger_conf_changed";
            for (auto& i : new_value) {
                auto it = old_value.find(i);
                sylar::Logger::ptr logger;
                if (it == old_value.end()) {
                    // 配置文件中新增logger
                    logger = SYLAR_LOG_NAME(i.name);    // SYLAR_LOG_NAME宏会自动将该logger加入管理类之中
                } else {
                    if (!(i == *it)) {  
                        //  配置文件中修改原先logger中的某些属性
                        // 注意因为重载<是基于name比较的，因此也可以认为looger的name也应该是唯一的才对
                        logger = SYLAR_LOG_NAME(i.name);                        
                    }
                }
                logger->setLevel(i.level);
                // std::cout << "** " << i.name << " level=" << i.level
                // << "  " << i.formatter << std::endl;
                if (!i.formatter.empty()) {
                    logger->setFormatter(i.formatter);      // 这里是以string的形式传递formatter
                }
            
                logger->clearAppenders();
                for(auto& a : i.appenders) {
                    sylar::LogAppender::ptr ap;
                    if (a.type == 1) {
                        ap.reset(new FileLogAppender(a.file));
                    } else if (a.type == 2) {
                        ap.reset(new StdoutLogAppender());
                    }
                    ap->setLevel(a.level);

                    logger->addAppender(ap);
                }
            }

            for (auto& i: old_value) {
                auto it = new_value.find(i);
                if (it == new_value.end()) {
                    // 配置文件中删除了原先的logger（不执行实际删除）
                    auto logger = SYLAR_LOG_NAME(i.name);
                    logger->setLevel((LogLevel::Level)100); // 设置很高的级别
                    logger->clearAppenders();
                }
            }    
        });
    }
};

static LogIniter __log_init;   // 该变量的构造函数会在main执行之前执行

std::string LoggerManager::toYamlString() {
    MutexType::Lock lock(m_mutex);
    YAML::Node node;
    for(auto& i : m_loggers) {
        node.push_back(YAML::Load(i.second->toYamlString()));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
}

void LoggerManager::init() {

}

}   // sylar
