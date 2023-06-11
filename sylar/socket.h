/**
 * @file socket.h
 * @brief Socket封装
 */
#ifndef __SYLAR_SOCKET_H__
#define __SYLAR_SOCKET_H__

#include <memory>
#include <netinet/tcp.h>
#include "address.h" 
#include "noncopyable.h"
namespace sylar {

class Socket : public std::enable_shared_from_this<Socket>, Noncopyable {
public:
    typedef std::shared_ptr<Socket> ptr;
    typedef std::weak_ptr<Socket> weak_ptr;     // 为啥要这个?

    /**
     * @brief Socket类型
     */
    enum Type {
        /// TCP类型
        TCP = SOCK_STREAM,
        /// UDP类型
        UDP = SOCK_DGRAM
    };
    /**
     * @brief Socket协议簇
     */
    enum Family {
        /// IPv4 socket
        IPv4 = AF_INET,
        /// IPv6 socket
        IPv6 = AF_INET6,
        /// Unix socket
        UNIX = AF_UNIX,
    };

    /**
     * @brief 创建TCP Socket(满足地址类型)
     * @param[in] address 地址
     */
     /// 因为地址对象本身就保存了它的family
    static Socket::ptr CreateTCP(sylar::Address::ptr address);

    /**
     * @brief 创建UDP Socket(满足地址类型)
     * @param[in] address 地址
     */
    static Socket::ptr CreateUDP(sylar::Address::ptr address);

    /**
     * @brief 创建IPv4的TCP Socket
     */
    static Socket::ptr CreateTCPSocket();

    /**
     * @brief 创建IPv4的UDP Socket
     */
    static Socket::ptr CreateUDPSocket();

    /**
     * @brief 创建IPv6的TCP Socket
     */
    static Socket::ptr CreateTCPSocket6();

    /**
     * @brief 创建IPv6的UDP Socket
     */
    static Socket::ptr CreateUDPSocket6();

    /**
     * @brief 创建Unix的TCP Socket
     */
    static Socket::ptr CreateUnixTCPSocket();

    /**
     * @brief 创建Unix的UDP Socket
     */
    static Socket::ptr CreateUnixUDPSocket();


    Socket(int family, int type, int protocol);
    ~Socket();

    int64_t getSendTimeout();
    void setSendTimeout(int64_t v);

    int64_t getRecvTimeout();
    void setRecvTimeout(int64_t);

    bool getOption(int level, int option, void* result, size_t* len);
    template<class T>   // T类型支持哪些
    bool getOption(int level, int option, T& result) {
        socklen_t length = sizeof(T);
        return getOption(level, option, &result, &length);
    }
    bool setOption(int level, int option, const void* result, size_t len);
    template<class T>
    bool setOption(int level, int option, const T& value) {
        return setOption(level, option, &value, sizeof(T));
    }
    
    bool bind(const Address::ptr addr);
    /// 接收connect链接
    Socket::ptr accept();
    bool connect(const Address::ptr addr, uint64_t timeout_ms = -1);

    bool listen(int backlog = SOMAXCONN);   // SOMAXCONN宏

    bool close();

    int send(const void* buffer, size_t length, int flags = 0);
    int send(const iovec* buffers, size_t length, int flags = 0);
    int sendTo(const void* buffer, size_t length, const Address::ptr to, int flags = 0);
    int sendTo(const iovec* buffers, size_t length, const Address::ptr to, int flags = 0);

    int recv(void* buffer, size_t length, int flags = 0);
    int recv(iovec* buffers, size_t length, int flags = 0);
    int recvFrom(void* buffer, size_t length, Address::ptr from, int flags = 0);
    int recvFrom(iovec* buffers, size_t length, Address::ptr from, int flags = 0);

    Address::ptr getRemoteAddress();
    Address::ptr getLocalAddress();

    int getFamily() const { return m_family;}
    int getType() const { return m_type;}
    int getProtocol() const { return m_protocol;}

    bool isConnected() const { return m_isConnected;}
    ///????
    bool isValid() const;
    int getError();

    /// 输出信息的流中
    std::ostream& dump(std::ostream& os) const;

    /// 返回socket句柄
    int getSocket() const { return m_sock;}

    // socket上的事件管理
    bool cancelRead();
    bool cancelWrite();
    bool cancelAccept();
    bool cancelAll();
    

protected:
    void initSock();

    void newSock();

    /**
     * @brief 通过fd的方式初始化sock套接字
     */
    bool init(int sock);
private:
    int m_sock;
    int m_family;
    int m_type;
    int m_protocol;

    bool m_isConnected;
    
    Address::ptr m_localAddress;
    Address::ptr m_remoteAddress;
};

}

#endif