# Sylar
C++高性能服务器框架

## 项目路径


## 日志系统

仿照Java Log4j日志框架实现
- `Logger`：定义日志类别
- `Formatter`: 日志格式
- `LogAppender`: 日志输出地(终端or文件)
> 观察者模式:Logger是被观察者，里面保存了所有观察者对象LogAppender
>
> 可以对比Log4cpp
