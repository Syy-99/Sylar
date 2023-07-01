# Sylar
C++高性能服务器框架

## 项目配置
boost：  sudo apt-get install libboost-dev
yaml-cpp ： sudo apt-get install libyaml-cpp-dev

流程图-uml图

## 项目路径


## 日志系统

## 配置系统




## 日志系统整合配置系统


## 线程模块




### 日志模块整合



### 配置模块整合



## 协程模块



### 协程接口





## 协程调度模块



### IO协程调度器

    
#### 定时器任务：Timer系统

## Socket相关

### Socket IO HooK




### 网络模版Socket


## 序列化byteArray



## HTTP协议封装

服务器框架需要提供一些协议的支持

服务器运行时，可能需要知道服务器的状态，因此给每个服务器提供一个HTTP接口，通过这个接口可以获得这个服务器当前运行的状态，并且通过HTTP接口修改这个服务器的状态

HTTP/1.1 - API

HttpRequest & HttpResponse;

- 参考： https://github.com/nodejs/http-parser/blob/main/http_parser.h

  - 因为我们需要解析到自己的结构体中，所以需要重写


- 需要将HTTP报文解析到我们自己的的结构体中

  - 使用ragel做HTTP解析, 可以从正规表达式生成可执行有限状态机,它可以生成C,C++,Object-C,D,Java和Ruby可执行代码

      https://blog.csdn.net/stayneckwind2/article/details/89285762 - 学习
      https://www.cnblogs.com/shanpow/p/4183215.html

      
      https://github.com/mongrel2/mongrel2

      ```
      int http_parser_init(http_parser *parser);
      int http_parser_finish(http_parser *parser);
      size_t http_parser_execute(http_parser *parser, const char *data, size_t len, size_t off);
      int http_parser_has_error(http_parser *parser);
      int http_parser_is_finished(http_parser *parser);
      ```
  - 难点： 需要修改一下它的rl文件，以符合我们的设计，每次解析都从data从头开始，以支持当缓存较小时可能需要分段解析
- 优势：

  - 支持cookie, session

  - 使用ragel做HTTP解析，速度更快

  - 支持chunk发送形式

## TCPServer封装

- 封装TCPServer, 并基于它实现了一个EchoServer

- 针对sock使用??

## Stream封装

封装流式bytearray的统一接口。将文件，socket封装成统一的接口。使用的时候，采用统一的风格操作。基于统一的风格，可以提供更灵活的扩展

- 针对文件/Socket进行的一层封装, 屏蔽两者的差异

read/write/readFixSize/WriteFixSize

## HttpSession/HttpConnection

Server.accept 的sokcet -> seesion  - Server端连接套接字
clinet.connect的socket -> connection - Client端连接套接字

- 优势： 提供封装，简化使用方式

  - 只用几行代码就可以发送HTTP请求了

HttpServer : TcpServer


HttpConnection 连接池 -> 管理长连接, 必须通过连接池获得的连接才可能是长连接

## HTTPServlet封装

Servlet?? 参考Java 

函数式Sevlet & ServletDispatch

- 一个请求进来会落到某个Servlet中处理, Server负责提供处理动作

  - 实际上就是HTTPServe在收到对某个uri的请求时,它的handleClient()就会执行它对应的Servlet中保存的方法（回调）

  - 目前只做了页面返回，实际应该也可以做文件下载之类的

- HTTPServer需要注册一个url, 来设定这个url可以响应那些，以及动作

- Servlet是一个虚拟接口

- 使用PostMan来调试HTTP服务器，模拟HTTP请求

## 性能测试

AB(apache bench)  <-- httpd-tools

同一个虚拟机性能比较, 返回相同404界面

- vs nginx: 17826 vs 17864  1000 Qps
- vs nginx  47    vs 43122  keep-live
- vs libevent 8900 vs 7000 vs 9000(nginx)

性能和nginx/libevent差不多

- 压测还取决于物理机性能，在我的虚拟机上跑不稳定

## 系统篇

### 守护进程

服务器程序出错也可以将程序重新拉起

deamon

fork() - 子进程：真正执行server的进程
         父进程： wati(pid), 如果子进程出现问题，则重新fork()

### 输入参数解析

int argc, char **argv

./sylar -d  -c conf 

- 守护进程 & 配置文件

### 环境变量
getenv
setenv

/proc/<pid>/cmdline|cwd|exe  找的和这个进程相关的东西

- cwd： 进程启动的路径
- exe: 执行的程序的真正路径(绝对路径)
- cmdline: 命令行参数
  - 结合全局变量构造函数，可以实现在main函数前解析参数

### 配置文件加载

配置的文件夹路径, 底下有一段yaml文件， log.yaml, tcp.yaml ....

- 启动时会将该文件夹下的yaml文件加载进来

  - 会递归加载这个路径下的所有目录

- 定时任务：没过一定周期就扫描文件是否变化，变化就重新加载，以实现实时加载


## Server主体框架

1. 防止server重复启动多次 -> 利用pid
2. 初始化日志文件路径: path/to/log
3. 工作文件路径: /path/to/work

4. 解析httpserver配置， 通过配置启动httpserver

## 相关知识

  c++宏：https://www.cnblogs.com/fnlingnzb-learner/p/6903966.html
  