#include "log.h"
#include <iostream>
#include <map>
#include <functional>
#include <time.h>

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
            os<<logger->getName();
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

    void Logger::log(LogLevel::Level level, LogEvent::ptr event) {
        if (level >= m_level) {
             auto self = shared_from_this();   // 返回一个当前类的std::share_ptr
            // 遍历每个目的地，根据日志级别来判断该日志是否可以输出到该目的地
            for (auto &it: m_appenders) {
                it->log(self, level, event);
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
        if (!appender->getFormatter()) {
            appender->setFormatter(m_formatter);
        }
        m_appenders.push_back(appender);
    }

    void Logger::delAppender(LogAppender::ptr appender) {
        for (auto it = m_appenders.begin(); it != m_appenders.end(); ++it) {
            if (*it == appender) {
                m_appenders.erase(it);
                break;
            }
        }
    }


    bool FileLogAppender::reopen() {
        if (m_filestream) {
            // 如果已经打开了文件，则需要先关闭
            m_filestream.close();
        }
        m_filestream.open(m_name);

        return !!m_filestream;   // 返回是否打开成功
    }

    void StdoutLogAppender::log(std::shared_ptr<Logger> logger,LogLevel::Level level, LogEvent::ptr event) {
        if (level >= m_level) {
            // 按格式构造日志信息，并输出到指定的日志输出地
            std::cout << m_formatter->format(logger, level, event);  // 输出到标准输出流中
        }

    }

    void FileLogAppender::log(std::shared_ptr<Logger> logger,LogLevel::Level level, LogEvent::ptr event) {
        if (level >= m_level) {
            // 按格式构造日志信息，并输出到指定的日志输出地
            // std::cout<<"file_appender log"<<std::endl;
            m_filestream << m_formatter->format(logger, level, event);   // 输出到文件流中
        }
    }

    FileLogAppender::FileLogAppender(const std::string &filename) : m_name(filename) {
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

}

Logger::ptr LoggerManager::getLogger(const std::string& name) {
    auto it = m_loggers.find(name);

    return it == m_loggers.end() ? m_root : it->second;

}

}   // sylar
