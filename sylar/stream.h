/**
 * @file stream.h
 * @brief 流接口
 */
#ifndef __SYLAR_STREAM_H__
#define __SYLAR_STREAM_H__

#include <memory>
#include "bytearray.h"

namespace sylar {

/**
 * @brief 流结构
 */
class Stream {
public:
    typedef std::shared_ptr<Stream> ptr;
    virtual ~Stream() {}


    virtual int read(void* buffer, size_t length) = 0;
    virtual int read(ByteArray::ptr ba, size_t length) = 0;
    /// 读固定长度的数据
    virtual int readFixSize(void* buffer, size_t length);
    virtual int readFixSize(ByteArray::ptr ba, size_t length);

    virtual int write(const void* buffer, size_t length) = 0;
    virtual int write(ByteArray::ptr ba, size_t length) = 0;
    virtual int writeFixSize(const void* buffer, size_t length);
    virtual int writeFixSize(ByteArray::ptr ba, size_t length);

    // 关闭流
    virtual void close() = 0;


private:

};

}


#endif