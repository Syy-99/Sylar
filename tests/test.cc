//
// Created by 沈衍羽 on 2023/4/30.
//

#include <iostream>
#include "../sylar/log.h"
#include "../sylar/util.h"

int main(int argc, char** argv) {
    sylar::Logger::ptr logger(new sylar::Logger);
    logger->addAppender(sylar::LogAppender::ptr(new sylar::StdoutLogAppender));


    sylar::FileLogAppender::ptr file_appender(new sylar::FileLogAppender("./log.txt"));

    sylar::LogFormatter::ptr fmt(new sylar::LogFormatter("%d%T%p%T%m%n"));  // 自定义格式器
    file_appender->setFormatter(fmt);  // 设置格式

    file_appender->setLevel(sylar::LogLevel::ERROR);    // 只有在错误比较严重的情况下才输出到文件日志中

    logger->addAppender(file_appender);

    // sylar::LogEvent::ptr event(new sylar::LogEvent(__FILE__, __LINE__, 0, sylar::GetThreadId(), sylar::GetFiberId(), time(0)));

    // logger->log(sylar::LogLevel::DEBUG, event);
    SYLAR_LOG_INFO(logger) << "test macro";
    SYLAR_LOG_ERROR(logger) << "test macro error";

    // SYLAR_LOG_FMT_ERROR(logger, "test macro fmt error %s", "aa");

    // 测试LoggerManager
    auto l = sylar::LoggerMgr::GetInstance()->getLogger("xx");
    SYLAR_LOG_INFO(l) << "xxx";
    std::cout<<"hello sylar log"<<std::endl;
    return 0;
}
