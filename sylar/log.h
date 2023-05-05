//#ifdef __SYLAR_LOG_H__
//#define __SYLAR_LOG_H__

#include <string>
#include <cstdint>
#include <list>
#include <sstream>
#include <fstream>
#include <memory>
#include <vector>

namespace sylar {

    class Logger;

    // 日志事件：保存该条日志的所有相关信息，会传递给日志器以进行日志写入
    class LogEvent {
    public:
        typedef std::shared_ptr<LogEvent> ptr;
        LogEvent(const char* file, int32_t line, uint32_t elapse, uint32_t thread_id, uint32_t fiber_id, uint64_t time);

        const char * getFile() const { return m_file; };
        int32_t getLine() const { return m_line; }
        uint32_t getElapse() const { return m_elapse; }
        uint32_t getThreadId() const { return m_threadId; }
        uint32_t getFiberId() const { return  m_fiberId; }
        uint64_t getTime() const { return m_time; }
        const std::string& getContent() const {return m_content; }
    private:
        const char *m_file = nullptr;   // 文件名
        int32_t m_line = 0;            // 行号
        uint32_t m_elapse = 0;          // 程序启动开始到现在的毫秒数，添加后，内存对齐
        uint32_t m_threadId = 0;        // 线程id
        uint32_t m_fiberId = 0;         // 协程id
        uint64_t m_time = 0;            // 时间戳
        std::string m_content;          // 日志内容

    };

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
    private:
        std::string m_pattern;  // 具体的格式
        std::vector<FormatItem::ptr> m_items;
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

    protected:  // 设置为protected, 子类可以使用下面的属性
        LogLevel::Level m_level = LogLevel::DEBUG;        // 日志输出地支持的最低日志级别

        LogFormatter::ptr m_formatter;  // 该输出地的日志日志输出格式

    };

    // 日志器: 日志信息输出的起始位置
    class Logger : public std::enable_shared_from_this<Logger>{
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

        LogLevel::Level getLevel() const { return m_level; }
        void setLevel(LogLevel::Level val) { m_level = val; }

        std::string getName() const { return m_name; }
    private:
        std::string m_name;                     // 日志器的名字
        LogLevel::Level m_level;                // 日志器支持的最低日志级别

        std::list<LogAppender::ptr> m_appenders;// 该日志器可以输出的目的地
        LogFormatter::ptr m_formatter;      // 该日志器输出的格式
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

}
//#endif