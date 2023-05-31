# Sylar
C++高性能服务器框架

## 项目配置
boost
yaml-cpp


流程图-uml图

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

> 难点：结构设计

> 需要好好理解逻辑！！！


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

> yaml序列化和反序列化 !!!
>
> 配置文件和log整合

## 线程模块

Thread: 封装线程类
线程用C++11线程
互斥量用pthread库

- 为什们不用C++ std::thread?

  - 优点： 跨平台，提供更多高级功能,比如future的
  - 缺点： 没有提供读写锁，读写没有分离
    - 高并发场景中，写少读多,如果只有普通的锁，导致性能下降

- 局部锁ScopedLock：

  有没有一种方法能实现“无论何处返回，都能自动unlock或destroy”的效果呢

  利用C++对象的生命周期（scope）结束时自动调用析构函数的机制，我们可以实现一种ScopedLock，也称为局部锁。


### 日志模块整合

多线程写日志，确保日志输出不串行

- 同步写日志，需要异步写日志吗？

> 难点： 整合，确定哪些地方需要加锁
>
> 下游加锁 

- 加互斥锁后，多线程写日志速度满，考虑使用自旋锁，避免进程切换，陷入内核等操作

  - 冲突时间较短，使用自旋锁性能更好

   - 不加锁100m, 互斥锁8M，自旋锁40M，原子锁40（自旋锁本身是靠CAS算法实现的）

   - 自旋锁基本上实现了高性能写

  
- bug: 写日志的时候，把文件删掉了，程序还在写

  - 修改： 每过一段时间reopen文件

### 配置模块整合

确保多个线程修改配置时的线程安全


## 协程模块

多线程，多协程框架

- 协程是用户控制切换的

- 如何理解协程? https://www.cnblogs.com/xcantaloupe/p/11102369.html

### 协程接口

封装assert宏

- assert只能显示在哪里出错，但是不会提示上层的函数在哪里被调用，不好定位具体逻辑

- 最好能显示调用堆栈 -> backtrace()方法


协程库 - 基于<ucontex.h>库

- 每个线程有一个主协程，所有的子协程都由主协程控制

  - Fiber::GetThis()
  
- 子协程无法创建子协程

- 难点：程序自行控制协程调度


Q: 
```c++
m_ctx.uc_link = nullptr;
```
不直接设置为主协程的原因： 为了可以重复利用协程的栈空间

- 导致问题，工作协程结束后，需要手动切换到主协程

## 协程调度模块

- 线程池, 每个线程有多个协程：N个线程运行M个协程

- 协程可以在线程之间切换，也可以绑定到指定线程运行

- 子协程可以通过向调度器添加调度任务的方式来运行另一个子协程。

```
          1 - N     1 - M
scheduler --> thread --> fiber

N: M: 1个协程调度器里面会有N线程，可以执行M个协程，这些协程会在线程中切换

创建调度器的线程理解为主线程，主线程会创建一个工作协程用来运行调度器
线程池中的每个线程都利用调度器的run函数，调度线程池中所有线程的协程

- 某些协程只允许被特定线程调用
```
1. 线程池：分配一组线程

2. 协程调度器： 将协程，指定到相应的线程上执行

  - 协程在空闲线程执行 or 协程在指定的线程执行


- 协程队列： 需要执行的工作

> 需要梳理逻辑

```
run()
1. 设置当前线程的scheduler ? 实际只有一个schduler吗?

2. 设置当前线程的run fiber

3. 协程调度循环while

  1. 协程消息队列里面是否有任务
  2. 无任务执行idle fiber
```

- 问题：锁的释放

  - 因为函数会调整到其他函数(协程)，导致持有锁的局部变量没有被释放，仍然需要手动释放锁 


> 协程调度模块需要好好梳理，有点问题，先继续

- 任务可以被任何线程中的任意一个协程执行
  
  实现的原理：

  - 调度器线程池中的所有线程都执行run方法

  - run方法负责从任务队列中取任务出来执行

    > 任务先被线程的run获得，然后线程会创建协程取执行该文件

    > 个人理解：这里每个线程都只有一个工作协程，并且只有该工作协程完成工作后才会释放，相当于一个单线程，

    > 一旦协程阻塞，整个线程都将阻塞


  理解user_callser

  - false：
    
    - main函数不参数调度，任务由线程池中的线程完成。这些线程有一个主协程，一个工作协程，一个睡眠协程， 
  
      ```
      没有任务： 主 <-> 睡眠， 循环切换，直到有任务
      有任务： 主 -> 工作，等待工作完成，在继续上面的操作
      ```

      - 当任务完成了，每个线程的run就结束，会自动销毁
      - 然后main中的stop()就会结束，最终main结束

  - true:

    - main函数所在线程也参数调度

    - 因为main函数线程还要负责main的运行，它的协程有：主协程，调度协程，睡眠协程，工作协程

    - main线程为了调度，需要切换到调度协程运行run

    - 因此在任务完成时，main线程处于调度协程中，因此还需要手动切换到主协程将stop()函数运行完成

    
>  目前的协程调度器实际上是不好的，因为在没有任务的时候，idle()协程一致占用CPU,但是没有做任何有意义的事情

### IO协程调度器

基于协程调度器+epoll，支持**异步IO**(???在哪体现??)


```
IOManager() 继承 Shceduler

```
- idle(): epoll_wait

- 不使用信号量来控制消息队列，使用管道通信

  - 有事件会通过管道来唤醒idle()，让线程继续工作

  - 优点： 避免使用信号量来阻塞整个线程，使得异步方法无法接收请求 ????
    
#### 定时器：Timer系统

- 注册定时任务；删除定时任务; 取消定时任务；

- 获取当前的定时器触发离现在的时间差

- 返回当前需要触发的定时器

定时器有什么用??

IO协程调度器的idle协程会在调度器空闲时阻塞在epoll_wait上，等待IO事件发生。在之前的代码里，epoll_wait具有固定的超时时间，这个值是5秒钟。加入定时器功能后，epoll_wait的超时时间改用当前定时器的最小超时时间来代替。epoll_wait返回后，根据当前的绝对时间把已超时的所有定时器收集起来，执行它们的回调函数


std::enable_shared_from_this 有什么意义？ - https://www.zhihu.com/question/30957800


注意点：基于系统时间，可能会修改时间，因此需要特殊处理

- 但是目前处理的思路比较粗暴，简单

为什们不用单调时间?  ->clock_gettime只能获得秒和纳秒