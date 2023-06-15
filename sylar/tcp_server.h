/**
 * @file tcp_server.h
 * @brief TCP服务器的封装
 */
#ifndef __SYLAR_TCP_SERVER_H__
#define __SYLAR_TCP_SERVER_H__

#include <memory>
#include <functional>

#include "iomanager.h"
#include "address.h"
#include "socket.h"
#include "noncopyable.h"
namespace sylar {

/**
* @brief TCP服务器封装
*/
class TcpServer : public std::enable_shared_from_this<TcpServer>
                    , Noncopyable {
public:
    typedef std::shared_ptr<TcpServer> ptr;
    TcpServer(sylar::IOManager* worker = sylar::IOManager::GetThis(), // IOManager需要提前初始化吧??
              sylar::IOManager* accept_worker = sylar::IOManager::GetThis());
    virtual ~TcpServer();

    virtual bool bind(sylar::Address::ptr addr);
    virtual bool bind(const std::vector<Address::ptr>& addrs,
                      std::vector<Address::ptr>& fails);

    /// 启动服务器
    virtual bool start();
    virtual void stop();

    uint64_t getRecvTimeout() const { return m_recvTimeout;}
    std::string getName() const { return m_name;}

    void setRecvTimeout(uint64_t v) { m_recvTimeout = v;}
    virtual void setName(const std::string& v) { m_name = v;}

    bool isStop() const { return m_isStop;}

protected:

    /// 处理用户连接, 每accept一次都会执行这个方法
    virtual void handleClient(Socket::ptr client);

    // 开始接受连接
    virtual void startAccept(Socket::ptr sock);
private:
    /// 监听Socket数组
    std::vector<Socket::ptr> m_socks;   // 支持同时监听多地址
    /// 新连接的Socket工作的调度器
    IOManager* m_worker;    

    /// 服务器Socket接收连接的调度器,
    IOManager* m_acceptWorker;

    /// 接收超时时间(毫秒)
    uint64_t m_recvTimeout;     // 连接超时时间

    /// 服务器名称
    std::string m_name;
    /// 服务器类型
    std::string m_type = "tcp";
    /// 服务是否停止
    bool m_isStop;
};

}




#endif
