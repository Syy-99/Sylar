//
// Created by 沈衍羽 on 2023/4/30.
//

#include <iostream>
#include "../sylar/log.h"
#include "../sylar/util.h"

int main(int argc, char** argv) {
    sylar::Logger::ptr logger(new sylar::Logger);
    logger->addAppender(sylar::LogAppender::ptr(new sylar::StdoutLogAppender));

    // sylar::LogEvent::ptr event(new sylar::LogEvent(__FILE__, __LINE__, 0, sylar::GetThreadId(), sylar::GetFiberId(), time(0)));

    // logger->log(sylar::LogLevel::DEBUG, event);
    SYLAR_LOG_INFO(logger) << "test macro";
    SYLAR_LOG_ERROR(logger) << "test macro error";

    // SYLAR_LOG_FMT_ERROR(logger, "test macro fmt error %s", "aa");
    std::cout<<"hello sylar log"<<std::endl;
    return 0;
}
