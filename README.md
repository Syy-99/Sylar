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

- 可以通过类图理解

## 配置系统

Config -> Yaml

- 支持复杂类型的Yaml， 配置的值可以允许vector,map<string, T>,set，自定义class

    - 简单类型： 用boost:lexical_cast内置转换方式

    - STL容器类型

    - 复杂类型： 提供自定义的解析方法 -> 序列化与反序列化

        - 自定义类型，需要实现实现sylar:LexicalCast的偏特化

        - 自定义类型和STL容器一起使用

    
    - Config::LookUp(key), key相同，但是类型不同，不会报错，需要处理? -> 解决，根据转换解决

        > 序列化是指把对象转换为字节序列
        > 
        > 反序列化是指把字节序列恢复为Java对象的过程：
        >
        > https://www.finclip.com/news/f/21276.html |  https://juejin.cn/post/7064942360106369054

- 配置系统的原则——约定优于配置： https://blog.csdn.net/ThinkWon/article/details/101703815，是一种软件设计范式。com/yanggb/p/10589073.html

    - 本质来说，系统、类库或框架应该假定合理的默认值，而非要求提供不必要的配置

    - 通过约定来指定可以配置的内容，并且只会配置这些内存

- 配置的事件机制： 配置变更后，代码级别如何感知做到实时变更？ 

    - 比如说，日志文件的路径发生变化，怎么办？

    - 当一个配置项发生修改的时候，可以反向通知对应的代码， 利用回调函数 -> 观察者模式

        - 通过添加回调函数，当配置被修改时，可以知晓


## 日志系统整合配置系统
```yaml
log:
    - name: root     # 日志名
      level:         # 这个日志记录的最低日志级别
      formatter:     # 这个日志模式的输出格式
      appender:      # 这个日志会输出的目的地
        - type:          #输出地的类型
          level:         #输出地接收的最低日志级别
          file:          #file日志输出的路径
    - name: root2     # 日志名
      level:         # 这个日志记录的最低日志级别
      formatter:     # 这个日志模式的输出格式
      appender:      # 这个日志会输出的目的地
        - type:          #输出地的类型
          level:         #输出地接收的最低日志级别
          file:          #file日志输出的路径
```

```c++
sylar::Logger g_logger = sylar::LoggerMgr::GetInstance()->getLogger(name);
SYLAR_LOG_INFO(g_logger)<<"xxxxx";
```
- 用户自定义的日志，如果有则返回，如果没有则创建

  - 如果自定义的日志在配置时，缺少了某些属性，则在log时，会调用root日志器执行写日志操作


```c++
// 定义LogDefine LogAppenderDefine + 偏特化LexicalCast， 实现日志配置解析
```

```c++
sylar::ConfigVar<std::set<LogDefine> >::ptr g_log_defines =
    sylar::Config::Lookup("logs", std::set<LogDefine>(), "logs config");
```
- 提供约定的日志器配置

- 当log下有配置文件时，通过loadYaml()执行对Log下的所有配置执行ConfigVar的反序列化函数，获得set<LogDefine>后，然后设置g_log_defines对象的值

- 在设置值时，会触发g_log_defines的回调函数，将所有的日志器加入日志管理对象中

- 最后，通过宏，根据用户传入的日志器名字，在日志管理对象中寻找是否有相应名称的日志器，执行日志输出操作


## 线程模块

Thread: 封装线程类
线程用C++11线程
互斥量用pthread库

- 为什们不用C++ std::thread?

  - 优点： 跨平台，提供更多高级功能,比如future的
  - 缺点： 没有提供读写锁，读写没有分离
    - 高并发场景中，写少读多,如果只有普通的锁，导致性能下降

Muted: 封装线程同步方式（锁）
