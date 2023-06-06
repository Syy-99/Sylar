
/**
 * @file address.h
 * @brief 网络地址的封装(IPv4,IPv6,Unix)
 */
#ifndef __SYLAR_ADDRESS_H__
#define __SYLAR_ADDRESS_H__

#include <memory>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>

namespace sylar {


class Address {
public:
    typedef std::shared_ptr<Address> ptr;
    virtual ~Address() {};

    /// 返回地址的协议族
    int getFamily() const;

    virtual const sockaddr* getAddr() const = 0;
    virtual socklen_t getAddrLen() const = 0

    virtual std::ostream& insert(std::ostream&os ) const = 0; 
    std::string toString();

    bool operator<(const Address& rhs) const;
    bool operator==(const Address& rhs) const;
    bool operator!=(const Address& rhs) const;
};

// IP地址的基类
class IPAddress : public Address {
public:
    typedef std::shared_ptr<IPAddress> ptr;

    // ???? 这一段函数怎么用的
    virtual IPAddress::ptr broadcastAddress(uint32_t prefix_len) = 0;
    virtual IPAddress::ptr networdAddress(uint32_t prefix_len) = 0;
    virtual IPAddress::ptr subnetMask(uint32_t prefix_len) = 0;

    virtual uint32_t getPort() const = 0;
    virtual void setPort() const = 0;
};


class IPv4Address : public IPAddress {
public:
    typedef std::shared_ptr<IPv4Address> ptr;

    /// IPv4地址初始化
    // 实际上ipv4的地址就是一个4字节的int
    IPv4Address(uint32_t address = INADDR_ANY, uint32_t port = 0);
    
    const sockaddr* getAddr() const override;
    socklen_t getAddrLen() const override;
    std::ostream& insert(std::ostream&os ) const override;

    IPAddress::ptr broadcastAddress(uint32_t prefix_len) override;
    IPAddress::ptr networdAddress(uint32_t prefix_len) override;
    IPAddress::ptr subnetMask(uint32_t prefix_len) override;

    uint32_t getPort() const override;
    void setPort() override;
private:
    sockaddr_in m_addr;
};

class IPv6Address : public Address {
public:
    typedef std::shared_ptr<IPv4Address> ptr;
    IPv6Address(uint32_t address = INADDR_ANY, uint32_t port = 0);
    
    const sockaddr* getAddr() const override;
    socklen_t getAddrLen() const override;
    std::ostream& insert(std::ostream&os ) const override;

    IPAddress::ptr broadcastAddress(uint32_t prefix_len) override;
    IPAddress::ptr networdAddress(uint32_t prefix_len) override;
    IPAddress::ptr subnetMask(uint32_t prefix_len) override;

    uint32_t getPort() const override;
    void setPort() override;
private:
    sockaddr_in6 m_addr;
};

class UnixAddress : public Address {
public:
    typedef std::shared_ptr<UnixAddress> ptr;
    UnixAddress(cosnt std::string& path);       // Unix套接字实际是一个文件
private:
}
}

#endif