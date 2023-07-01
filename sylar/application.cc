#include "application.h"
#include "log.h"
#include "env.h"
#include "config.h"
#include "daemon.h"

namespace sylar {

static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

static sylar::ConfigVar<std::string>::ptr g_server_work_path =
    sylar::Config::Lookup("server.work_path"
            ,std::string("/home/gaoxiaoyu/syy/Sylar")       // 工作路径
            , "server work path");

static sylar::ConfigVar<std::string>::ptr g_server_pid_file =
    sylar::Config::Lookup("server.pid_file"
            ,std::string("sylar.pid")   // pid文件命名
            , "server pid file");

struct HttpServerConf {
    std::vector<std::string> address;
    int keepalive = 0;
    int timeout = 100 * 2 * 60;
    std::string name;

    bool isValid() const {
        return !address.empty();
    }

    bool operator==(const HttpServerConf& oth) const {
        return address == oth.address
            && keepalive == oth.keepalive
            && timeout == oth.timeout
            && name == oth.name;
    }
};
template<>
class LexicalCast<std::string, HttpServerConf> {
public:
    HttpServerConf operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        HttpServerConf conf;
        conf.keepalive = node["keepalive"].as<int>(conf.keepalive);
        conf.timeout = node["timeout"].as<int>(conf.timeout);
        conf.name = node["name"].as<std::string>(conf.name);
        if(node["address"].IsDefined()) {
            for(size_t i = 0; i < node["address"].size(); ++i) {
                conf.address.push_back(node["address"][i].as<std::string>());
            }
        }
        return conf;
    }
};

template<>
class LexicalCast<HttpServerConf, std::string> {
public:
    std::string operator()(const HttpServerConf& conf) {
        YAML::Node node;
        node["name"] = conf.name;
        node["keepalive"] = conf.keepalive;
        node["timeout"] = conf.timeout;
        for(auto& i : conf.address) {
            node["address"].push_back(i);
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

static sylar::ConfigVar<std::vector<HttpServerConf> >::ptr g_http_servers_conf
    = sylar::Config::Lookup("http_servers", std::vector<HttpServerConf>(), "http server config");\

Application* Application::s_instance = nullptr;
Application::Application() {
    s_instance = this;
}

// 服务的所有初始化操作
bool Application::init(int argc, char** argv) {
    SYLAR_LOG_ERROR(g_logger) << "Application::init";
    m_argc = argc;
    m_argv = argv;

    sylar::EnvMgr::GetInstance()->addHelp("s", "start with the terminal");
    sylar::EnvMgr::GetInstance()->addHelp("d", "run as daemon");
    sylar::EnvMgr::GetInstance()->addHelp("c", "conf path default: ./conf");    // 配置文件路径
    sylar::EnvMgr::GetInstance()->addHelp("p", "print help");

    bool is_print_help = false;
    if(!sylar::EnvMgr::GetInstance()->init(argc, argv)) {   
        // 命令行，参数解析失败，则无法执行，需要打印帮助信息
        is_print_help = true;
    }
    if(sylar::EnvMgr::GetInstance()->has("p")) {
        // 命令行参数中有p, 则需要打印帮助信息
        is_print_help = true;
    }
    if(is_print_help) {
        sylar::EnvMgr::GetInstance()->printHelp();
        return false;
    }

    int run_type = 0;       // 记录以何种方式运行
    if(sylar::EnvMgr::GetInstance()->has("s")) {
        run_type = 1;
    }
    if(sylar::EnvMgr::GetInstance()->has("d")) {
        run_type = 2;
    }
    if(run_type == 0) {
        sylar::EnvMgr::GetInstance()->printHelp();
        return false;
    }

    // 获得配置文件的绝对路径
    std::string conf_path = sylar::EnvMgr::GetInstance()->getAbsolutePath(
        sylar::EnvMgr::GetInstance()->get("c", "conf")
    );
    SYLAR_LOG_INFO(g_logger) << "load conf path:" << conf_path;
    // 加载配置文件
    sylar::Config::LoadFromConfDir(conf_path);

    // 检查是否已经存在pid文件
    std::string pidfile = g_server_work_path->getValue()
                                    + "/" + g_server_pid_file->getValue();
    if(sylar::FSUtil::IsRunningPidfile(pidfile)) {
        // 如果有且文件中记录的pid存在，则说明服务器程序已经运行
        SYLAR_LOG_ERROR(g_logger) << "server is running:" << pidfile;
        return false;
    }

    // 如果没有，则创建该文件
    if(!sylar::FSUtil::Mkdir(g_server_work_path->getValue())) {
        SYLAR_LOG_FATAL(g_logger) << "create work path [" << g_server_work_path->getValue()
            << " errno=" << errno << " errstr=" << strerror(errno);
        return false;
    }

    return true;        // 返回true， 说明可以启动，没有出现问题

}

bool Application::run() {
    std::cout<<"run "<<std::endl;
    SYLAR_LOG_INFO(g_logger) << " Application::run()";
    bool is_daemon = sylar::EnvMgr::GetInstance()->has("d");
    return start_daemon(m_argc, m_argv,
                    std::bind(&Application::main, this, std::placeholders::_1,std::placeholders::_2), 
                        is_daemon);
}

// 真正执行的main()函数，是通过start_deamon fork()出来的进程执行的函数
int Application::main(int argc, char** argv) {
    SYLAR_LOG_INFO(g_logger) << "Application::main";        // ?? 为什么不显示
    // std::cout<<"Application::main "<<std::endl;
    // 向pid文件中记录服务器进程的pid
    std::string pidfile = g_server_work_path->getValue()
                                + "/" + g_server_pid_file->getValue();
    std::ofstream ofs(pidfile);
    if(!ofs) {
        SYLAR_LOG_INFO(g_logger) << "open pidfile " << pidfile << " failed";
        return false;
    }
    ofs << getpid();

    // auto http_confs = g_http_servers_conf->getValue();
    // for (auto& i : http_confs) {
    //     std::cout<<LexicalCast<HttpServerConf, std::string>()(i)<<std::endl;
    // }

    sylar::IOManager iom(1);
    iom.schedule(std::bind(&Application::run_fiber, this));
    iom.stop();
    return 0;
}

int Application::run_fiber() {
    auto http_confs = g_http_servers_conf->getValue();   // 获得服务器的配置
    for(auto& i : http_confs) {

        std::vector<Address::ptr> address;
        for(auto& a : i.address) {
            // 检查是否提供端口号 - 只针对ip地址
            size_t pos = a.find(":");
            if(pos == std::string::npos) {
                SYLAR_LOG_ERROR(g_logger) << "invalid address: " << a;
                // address.push_back(UnixAddress::ptr(new UnixAddress(a)));
                continue;
            }

            auto addr = sylar::Address::LookupAny(a);   // 尝试取ip地址
            if(addr) {
                address.push_back(addr);
                continue;
            }
            
            // 取不到，则直接通过网卡获得地址
            std::vector<std::pair<Address::ptr, uint32_t> > result;
            if(!sylar::Address::GetInterfaceAddresses(result, a.substr(0, pos))) {
                SYLAR_LOG_ERROR(g_logger) << "invalid address: " << a;
            }

            for(auto& x : result) {
                auto ipaddr = std::dynamic_pointer_cast<IPAddress>(x.first);
                if(ipaddr) {
                    ipaddr->setPort(atoi(a.substr(pos + 1).c_str()));
                }
                address.push_back(ipaddr);
            }
        }

        sylar::http::HttpServer::ptr server(new sylar::http::HttpServer(i.keepalive));
        std::vector<Address::ptr> fails;
        if(!server->bind(address, fails)) {     // bind地址
            for(auto& x : fails) {
                SYLAR_LOG_ERROR(g_logger) << "bind address fail:"
                    << *x;
                    
            }
            _exit(0);   // 绑定失败，直接退出
        }
        if (!i.name.empty())
            server->setName(i.name);        // ??? 没有生效呀????
        server->start();    // 启动这个server?? 可以启动多个server吗???
        m_httpservers.push_back(server);

    }  
    return 0;
}

}