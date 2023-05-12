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

## 配置系统

Config -> Yaml

- 支持复杂类型的Yaml， 配置的值可以允许vector,map<string, T>,set，自定义class

    - 简单类型： 用boost:lexical_cast内置转换方式

    - 复杂类型： 提供自定义的解析方法 -> 序列化与反序列化
    
    - Config::LookUp(key), key相同，但是类型不同，不会报错，需要处理?

        - 利用模版偏特例化

        > 序列化是指把对象转换为字节序列
        > 
        > 反序列化是指把字节序列恢复为Java对象的过程：
        >
        > https://www.finclip.com/news/f/21276.html |  https://juejin.cn/post/7064942360106369054

- 配置系统的原则——约定优于配置： https://blog.csdn.net/ThinkWon/article/details/101703815，是一种软件设计范式。com/yanggb/p/10589073.html

    - 本质来说，系统、类库或框架应该假定合理的默认值，而非要求提供不必要的配置

    - 通过约定来减少配置

    - 只会配置约定的参数