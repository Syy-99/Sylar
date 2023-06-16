/**
 * @file socket_stream.h
 * @brief Socket流式接口封装
 ???? 这个封装有啥用呀???
 */
#ifndef __SYLAR_SOCKET_STREAM_H__
#define __SYLAR_SOCKET_STREAM_H__

#include "stream.h"
#include "socket.h"

namespace sylar {
// 之前的socket.h封装是对C接口的封装
// 现在的封装是业务级的接口封装

class SocketStream : public Stream {
public:
    typedef std::shared_ptr<SocketStream> ptr;
    SocketStream(Socket::ptr sock, bool owner = true);
    ~SocketStream();

    virtual int read(void* buffer, size_t length) override;
    virtual int read(ByteArray::ptr ba, size_t length) override;

    virtual int write(const void* buffer, size_t length) override;
    virtual int write(ByteArray::ptr ba, size_t length) override;

    // 关闭流
    virtual void close() = 0;

    Socket::ptr getSocket() const { return m_socket;}

    /// 返回是否连接
    bool isConnected() const;
protected:
     /// Socket类
    Socket::ptr m_socket;       // 管理的Socket

    /// 是否主控
    bool m_owner;       // 是否全权交给; 如果不是，则说明只是做操作
};
}


#endif