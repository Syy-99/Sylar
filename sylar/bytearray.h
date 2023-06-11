/**
 * @file bytearray.h
 * @brief 二进制数组(序列化/反序列化)
 */
#ifndef __SYLAR_BYTEARRAY_H__
#define __SYLAR_BYTEARRAY_H__

#include <memory>
#include <string>
#include <stdint.h>
namespace sylar {

/**
 * @brief 二进制数组,提供基础类型的序列化,反序列化功能
 */
class ByteArray {
public:
    typedef std::shared_ptr<ByteArray> ptr;

    /**
     * @brief ByteArray的存储节点,使用链表管理
     */
    struct Node {
        Node(size_t s);
        Node();
        ~Node();

        char *ptr;
        Node* next;
        size_t size;
    };

    /**
     * @brief 使用指定长度的内存块构造ByteArray
     * @param[in] base_size 内存块大小
     */
    ByteArray(size_t base_size = 4096);     // base_size: 每个链表的长度
    ~ByteArray();

    // 提供两种格式的写： 不压缩 of 压缩
    // 写固定长度的数据
    void writeFint8(const int8_t value);
    void writeFuint8(const uint8_t value);
    void writeFint16(const int16_t value);
    void writeFuint16(const uint16_t value);
    void writeFint32(const int32_t value);
    void writeFuint32(const uint32_t value);
    void writeFint64(const int64_t value);
    void writeFuint64(const uint64_t value);

    // void writeInt8(const int8_t value);
    // void writeUint8(const uint8_t value);  一个字节无意义
    // void writeInt16(const int16_t value);
    // void writeUint16(const uint16_t value);  两个字节为什么也无意义???
    void writeInt32(const int32_t value);
    void writeUint32(const uint32_t value);
    void writeInt64(const int64_t value);
    void writeUint64(const uint64_t value);

    void writeFloat(float value);
    void writeDouble(double value)

    // 写入string类型，需要写长度,F16表示长度的位
    void writeStringF16(const std::string& value);
    void writeStringF32(const std::string& value);
    void writeStringF64(const std::string& value);
    // 写入std::string类型的数据,用无符号Varint64作为长度类型(压缩的长度)
    void writeStringVint(const std::string& value);

    void writeStringWithoutLength(const std::string& value);


    // read
    int8_t  readFint8();
    uint8_t readFuint16();
    int16_t readFint16();
    int16_t readFuint16();
    int32_t readFint32();
    int32_t readFuint32();
    int64_t readFint64();
    int64_t readFuint64();

    int32_t     readInt32();
    uint32_t    readUint32();
    int64_t     readInt64();
    uint64_t    readUint64();

    float readFloat();
    double readDouble();

    std::string readStringF16();
    std::string readStringF32();
    std::string readStringF64();
    std::string readStringVint();
    

    // 内部操作
    void clear();

    void write(cosnt void* buf, size_t size);
    void read(char* buf, size_t size);

    size_t getPosition() const { retunr m_position;}
    void setPosition(size_t v);

    bool writeToFile(const std::string& name) const;
    bool readFromFile(const std::string& name);

    // 返回内存块的大小
    size_t getBaseSize() const { return m_baseSize;}

    // 返回可读取数据大小
    size_t getReadSize() const { return m_size - m_position;}


    bool isLittleEndian() const;
    void setIsLittleEndian(bool val);
private:

    /**
     * @brief 扩容ByteArray,使其可以容纳size个数据(如果原本可以可以容纳,则不扩容)
     */
    void addCapacity(size_t size);

    /**
     * @brief 获取当前的可写入容量
     */
    size_t getCapacity() const { return m_capacity - m_position;}
private:

    /// 内存块的大小(每个node有多大)
    size_t m_baseSize;
    /// 当前操作位置
    size_t m_position;
    /// 当前的总容量
    size_t m_capacity;
    /// 当前真实数据的大小
    size_t m_size;

    /// 字节序,默认大端
    int8_t m_endian;

    /// 第一个内存块指针
    Node* m_root;
    /// 当前操作的内存块指针
    Node* m_cur;

};

}





#endif