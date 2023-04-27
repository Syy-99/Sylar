//#ifdef __SYLAR_LOG_H__
//#define __SYLAR_LOG_H__

#include <string>
#include <cstdint>

namespace sylar {

    // 日志事件：保存该条日志的所有相关信息，会传递给日志器以进行日志写入
    class LogEvent {
    public:
        typedef std::shared_ptr<LogEvent> ptr;
        LogEvent();
    private:
        const char *m_file = nullptr;   // 文件名
        uint32_t m_line = 0;            // 行号
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
            DEBUG = 1,
            INFO = 2,
            WARN = 3,
            ERROR = 4,
            FATAL = 5,
        };
    };

    // 日志格式器：控制日志写入的格式
    class LogFormatter{
    public:
        typedef std::shared_ptr<LogFormatter> ptr;

        // 控制event中内容的输出格式
        std::string format(LogEvent::ptr event);
    private:
    };

    // 日志输出地
    class LogAppender{
    public:
        typedef std::shared_ptr<LogAppender> ptr;
        virtual ~LogAppender() {};  // 因为可能有多种日志输出地（终端or文件），因此需要定义为虚基类

        // 输出到指定的目的地
        void log(LogLevel::Level level, LogEvent::ptr event);
    private:
        LogLevel::Level m_level;    // 日志输出地支持的最低日志级别

    };

    // 日志器
    class Logger {
    public:
        typedef std::shared_ptr<Logger> ptr;

        Logger(const std::string& name = "root");

        void log(LogLevel::Level level, LogEvent::ptr event);
    private:
        std::string m_name;     // 日志器的名字
        LogLevel::Level m_level;    // 由该日志器输出的日志的级别
        LogAppender::ptr p;
    };

    // 具体的日志输出地——控制台
    class StdoutLogAppender : public LogAppender {

    };

    // 具体的日志输出地——文件
    class FileLogAppender : public LogAppender {

    };

}
//#endif