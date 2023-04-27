#include <iostream>
#include "log.h"

sylar::Logger::Logger(const std::string &name) : m_name(name) {

}

void sylar::Logger::log(sylar::LogLevel::Level level, sylar::LogEvent::ptr event) {
    if (level >= m_level) {
        // 遍历每个目的地，根据日志级别来判断该日志是否可以输出到该目的地
        for (auto &it : m_appenders) {
            it->log(level, event);
        }
    }
}

void sylar::Logger::debug(sylar::LogEvent::ptr event) {
    log(LogLevel::DEBUG, event);
}

void sylar::Logger::info(sylar::LogEvent::ptr event) {
    log(LogLevel::INFO, event);
}

void sylar::Logger::warn(sylar::LogEvent::ptr event) {
    log(LogLevel::WARN, event);
}

void sylar::Logger::error(sylar::LogEvent::ptr event) {
    log(LogLevel::ERROR, event);
}

void sylar::Logger::fatal(sylar::LogEvent::ptr event) {
    log(LogLevel::FATAL, event);
}

void sylar::Logger::addAppender(sylar::LogAppender::ptr appender) {
    m_appenders.push_back(appender);
}

void sylar::Logger::delAppender(sylar::LogAppender::ptr appender) {
    for (auto it = m_appenders.begin(); it != m_appenders.end(); ++it) {
        if (*it == appender) {
            m_appenders.erase(it);
            break;
        }
    }
}


bool sylar::FileLogAppender::reopen() {
    if (m_filestream) {
        // 如果已经打开了文件，则需要先关闭
        m_filestream.close();
    }
    m_filestream.open(m_name);

    return !!m_filestream   // 返回是否打开成功
}

void sylar::StdoutLogAppender::log(sylar::LogLevel::Level level, sylar::LogEvent::ptr event) {
    if (level >= m_level) {
        // 按格式构造日志信息，并输出到指定的日志输出地
        std::cout<<m_formatter->format(event);  // 输出到标准输出流中
    }

}

void sylar::FileLogAppender::log(sylar::LogLevel::Level level, sylar::LogEvent::ptr event) {
    if (level >= m_level) {
        // 按格式构造日志信息，并输出到指定的日志输出地
        m_filestream<<m_formatter->format(event);   // 输出到文件流中
    }
}

sylar::FileLogAppender::FileLogAppender(const std::string &filename) : m_name(filename){

}
