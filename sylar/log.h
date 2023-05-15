#ifndef __SYLAR_LOG_H__
#define __SYLAR_LOG_H__

#include <string>
#include <cstdint>
#include <list>
#include <sstream>
#include <fstream>
#include <memory>
#include <vector>
#include <stdarg.h>
#include <map>
#include "util.h"
#include "singleton.h"

/**
 * @brief 使用流式方式将日志级别level的日志写入到logger
 */
#define SYLAR_LOG_LEVEL(logger, level) \
    if(logger->getLevel() <= level) \
        sylar::LogEventWrap(sylar::LogEvent::ptr(new sylar::LogEvent(logger, level, \
                        __FILE__, __LINE__, 0, sylar::GetThreadId(),\
                sylar::GetFiberId(), time(0)))).getSS()

/**
 * @brief 使用流式方式将日志级别debug的日志写入到logger
 */
#define SYLAR_LOG_DEBUG(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::DEBUG)

/**
 * @brief 使用流式方式将日志级别info的日志写入到logger
 */
#define SYLAR_LOG_INFO(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::INFO)

/**
 * @brief 使用流式方式将日志级别warn的日志写入到logger
 */
#define SYLAR_LOG_WARN(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::WARN)

/**
 * @brief 使用流式方式将日志级别error的日志写入到logger
 */
#define SYLAR_LOG_ERROR(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::ERROR)

/**
 * @brief 使用流式方式将日志级别fatal的日志写入到logger
 */
#define SYLAR_LOG_FATAL(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::FATAL)



/**
 * @brief 使用格式化方式将日志级别level的日志写入到logger
 */
#define SYLAR_LOG_FMT_LEVEL(logger, level, fmt, ...) \
    if(logger->getLevel() <= level) \
        sylar::LogEventWrap(sylar::LogEvent::ptr(new sylar::LogEvent(logger, level, \
                        __FILE__, __LINE__, 0, sylar::GetThreadId(),\
                sylar::GetFiberId(), time(0)))).getEvent()->format(fmt, __VA_ARGS__)

 
/**
 * @brief 使用格式化方式将日志级别debug的日志写入到logger
 */
#define SYLAR_LOG_FMT_DEBUG(logger, fmt, ...) SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::DEBUG, fmt, __VA_ARGS__)

/**
 * @brief 使用格式化方式将日志级别info的日志写入到logger
 */
#define SYLAR_LOG_FMT_INFO(logger, fmt, ...)  SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::INFO, fmt, __VA_ARGS__)

/**
 * @brief 使用格式化方式将日志级别warn的日志写入到logger
 */
#define SYLAR_LOG_FMT_WARN(logger, fmt, ...)  SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::WARN, fmt, __VA_ARGS__)

/**
 * @brief 使用格式化方式将日志级别error的日志写入到logger
 */
#define SYLAR_LOG_FMT_ERROR(logger, fmt, ...) SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::ERROR, fmt, __VA_ARGS__)

/**
 * @brief 使用格式化方式将日志级别fatal的日志写入到logger
 */
#define SYLAR_LOG_FMT_FATAL(logger, fmt, ...) SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::FATAL, fmt, __VA_ARGS__)
               
// 获得root日志器
#define SYLAR_LOG_ROOT() sylar::LoggerMgr::GetInstance()->getRoot()

#define SYLAR_LOG_NAME(name) sylar::LoggerMgr::GetInstance()->getLogger(name)

namespace sylar {

    class Logger;
    class LoggerManager;

    // 日志级别
    class LogLevel {
    public:
        enum Level {
            UNKNOW = 0,
            DEBUG = 1,
            INFO = 2,
            WARN = 3,
            ERROR = 4,
            FATAL = 5,
        };

        static const char * ToString(LogLevel::Level level);
        static LogLevel::Level FromString(const std::string& str);
    };

    // 日志事件：保存该条日志的所有相关信息，会传递给日志器以进行日志写入
    class LogEvent {
    public:
        typedef std::shared_ptr<LogEvent> ptr;
        LogEvent(std::shared_ptr<Logger> logger,LogLevel::Level level, 
                 const char* file, int32_t line, uint32_t elapse, 
                 uint32_t thread_id, uint32_t fiber_id, uint64_t time);

        const char * getFile() const { return m_file; };
        int32_t getLine() const { return m_line; }
        uint32_t getElapse() const { return m_elapse; }
        uint32_t getThreadId() const { return m_threadId; }
        uint32_t getFiberId() const { return  m_fiberId; }
        uint64_t getTime() const { return m_time; }
        std::string getContent() const {return m_ss.str(); }
        std::shared_ptr<Logger> getLogger() const { return m_logger; }
        LogLevel::Level getLevel() const { return m_level; }

        std::stringstream& getSS() { 
            return m_ss; 
        }

       void format(const char* fmt, ...);

        /**
        * @brief 格式化写入日志内容
        */
        void format(const char* fmt, va_list al);
    private:
        const char *m_file = nullptr;   // 文件名
        int32_t m_line = 0;            // 行号
        uint32_t m_elapse = 0;          // 程序启动开始到现在的毫秒数，添加后，内存对齐
        uint32_t m_threadId = 0;        // 线程id
        uint32_t m_fiberId = 0;         // 协程id
        uint64_t m_time = 0;            // 时间戳
        // std::string m_content;          // 日志内容
        /// 日志内容流
        std::stringstream m_ss;
        
        /// 日志器
        std::shared_ptr<Logger> m_logger;
        /// 日志等级
        LogLevel::Level m_level;

    };

    /**
    * @brief 日志事件包装器
        使用waper,使得在调用宏输出结束后，也会被析构
    */
    class LogEventWrap {
    public:

        /**
        * @brief 构造函数
        * @param[in] e 日志事件
        */
        LogEventWrap(LogEvent::ptr e);

        /**
        * @brief 析构函数
        */
        ~LogEventWrap();

        /**
        * @brief 获取日志事件
        */
        LogEvent::ptr getEvent() const { return m_event;}

        /**
        * @brief 获取日志内容流
        */
        std::stringstream& getSS();
    private:
        /**
        * @brief 日志事件
        */
        LogEvent::ptr m_event;
    };


    // 日志格式器：控制日志写入的格式
    class LogFormatter{
    public:
        typedef std::shared_ptr<LogFormatter> ptr;
        LogFormatter(const std::string& pattern);
        // 根据设置的格式，解析event中内容, 获得最终输出的日志信息
        std::string format(std::shared_ptr<Logger> logger, LogLevel::Level level,LogEvent::ptr event);
    public:
        class FormatItem {
        public:
            typedef std::shared_ptr<FormatItem> ptr;
            virtual ~FormatItem() {};
            // 输出解析m_pattern后的每个格式对应信息
            virtual void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;
        };

        // 解析m_pattern
        void init();

        bool isError() const { return m_error; }

    private:
        std::string m_pattern;  // 具体的格式
        std::vector<FormatItem::ptr> m_items;   // 具体的每个格式对应的项

        bool m_error = false;   // 判断是否m_pattern是否有效
    };

    // 日志输出地
    class LogAppender{
    public:
        typedef std::shared_ptr<LogAppender> ptr;
        virtual ~LogAppender() {};  // 因为可能有多种日志输出地（终端or文件），因此需要定义为虚基类

        // 输出到指定的目的地
        virtual void log(std::shared_ptr<Logger> logger,LogLevel::Level level, LogEvent::ptr event) = 0;

        void setFormatter(LogFormatter::ptr val) { m_formatter = val; }
        LogFormatter::ptr getFormatter() const { return m_formatter; }

        /**
        * @brief 获取日志级别
        */
        LogLevel::Level getLevel() const { return m_level;}

        /**
        * @brief 设置日志级别
        */
        void setLevel(LogLevel::Level val) { m_level = val;}
    protected:  // 设置为protected, 子类可以使用下面的属性
        LogLevel::Level m_level = LogLevel::DEBUG;        // 日志输出地支持的最低日志级别

        LogFormatter::ptr m_formatter;  // 该输出地的日志日志输出格式

    };

    // 日志器: 日志信息输出的起始位置
    class Logger : public std::enable_shared_from_this<Logger>{
        friend class LoggerManager;
    public:
        typedef std::shared_ptr<Logger> ptr;

        explicit Logger(const std::string& name = "root");

        void log(LogLevel::Level level, LogEvent::ptr event);

        void debug(LogEvent::ptr event);    // 输出debug级别的日志
        void info(LogEvent::ptr event);
        void warn(LogEvent::ptr event);
        void error(LogEvent::ptr event);
        void fatal(LogEvent::ptr event);

        void addAppender(LogAppender::ptr appender);
        void delAppender(LogAppender::ptr appender);
        void clearAppenders();

        LogLevel::Level getLevel() const { return m_level; }
        void setLevel(LogLevel::Level val) { m_level = val; }

        std::string getName() const { return m_name; }

        void setFormatter(LogFormatter::ptr val);
        void setFormatter(const std::string& val);
        LogFormatter::ptr getFormatter() const { return m_formatter; }
    private:
        std::string m_name;                     // 日志器的名字
        LogLevel::Level m_level;                // 日志器支持的最低日志级别，默认是DEBUG级别

        std::list<LogAppender::ptr> m_appenders;// 该日志器可以输出的目的地，只能手动添加
        LogFormatter::ptr m_formatter;          // 该日志器输出的格式

        /// 主日志器
        Logger::ptr m_root;     // 每个日志器都可能需要访问root日志器，以读取配置
    };

    // 具体的日志输出地——控制台
    class StdoutLogAppender : public LogAppender {
    public:
        typedef std::shared_ptr<StdoutLogAppender> ptr;
        void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override;
    private:
    };

    // 具体的日志输出地——文件
    class FileLogAppender : public LogAppender {
    public:
        typedef std::shared_ptr<FileLogAppender> ptr;
        FileLogAppender(const std::string& filename);
        void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override;

        bool reopen();  // 重新打开文件
    private:
        std::string m_name;
        std::ofstream m_filestream;     // 文件输出流
    };


  /**
 * @brief 日志器管理类, 负责管理所有的logger
 */
class LoggerManager {
public:
    LoggerManager();
    Logger::ptr getLogger(const std::string& name);

    void init();

    Logger::ptr getRoot() const { return m_root; }
private:

    // 日志器容器
    std::map<std::string, Logger::ptr> m_loggers;
    // 默认的主日志器, 写到控制台
    Logger::ptr m_root;

};  

/// 日志器管理类单例模式
typedef sylar::Singleton<LoggerManager> LoggerMgr;

}
#endif