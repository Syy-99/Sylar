#include "bytearray.h"

namespace sylar {

ByteArray::Node::Node(size_t s) : ptr(new char[s]), size(s), next(nullptr){}
ByteArray::Node::Node() : ptr(nullptr), size(0), next(nullptr) {}

ByteArray::Node::~Node() {
    if (ptr)
        delete[] ptr;
}


ByteArray::ByteArray(size_t base_size)
    : m_baseSize(base_size)
    , m_position(0)
    , m_capacity(base_size)
    , m_size(0)
    , m_root(new Node(base_size))
    , m_cur(m_root)
    , m_endian(SYLAR_BIG_ENDIAN) {

}
ByteArray::~ByteArray() {
    /// 释放链表
    Node* tmp = m_root;
    while(tmp) {
        m_cur = tmp;
        tmp = tmp->next;
        delete m_cur;
    } 
}

bool isLittleEndian() const {
    return m_endian == SYLAR_LITTLE_ENDIAN;
}
void setIsLittleEndian(bool val) {
    if(val) {
        m_endian = SYLAR_LITTLE_ENDIAN;
    } else {
        m_endian = SYLAR_BIG_ENDIAN;
    }
}


// 提供两种格式的写： 不压缩 of 压缩
// 写固定长度的数据
// ??? 感觉可以用模版方法实现吧
void ByteArray::writeFint8  (int8_t value) {
    write(&value, sizeof(value));
}

void ByteArray::writeFuint8 (uint8_t value) {
    write(&value, sizeof(value));
}
void ByteArray::writeFint16 (int16_t value) {
    if(m_endian != SYLAR_BYTE_ORDER) {      // 网络字节序和机器字节序不同
        value = byteswap(value);    // 改变字节序
    }   
    write(&value, sizeof(value));
}

void ByteArray::writeFuint16(uint16_t value) {
    if(m_endian != SYLAR_BYTE_ORDER) {
        value = byteswap(value);
    }
    write(&value, sizeof(value));
}

void ByteArray::writeFint32 (int32_t value) {
    if(m_endian != SYLAR_BYTE_ORDER) {
        value = byteswap(value);
    }
    write(&value, sizeof(value));
}

void ByteArray::writeFuint32(uint32_t value) {
    if(m_endian != SYLAR_BYTE_ORDER) {
        value = byteswap(value);
    }
    write(&value, sizeof(value));
}

void ByteArray::writeFint64 (int64_t value) {
    if(m_endian != SYLAR_BYTE_ORDER) {
        value = byteswap(value);
    }
    write(&value, sizeof(value));
}

void ByteArray::writeFuint64(uint64_t value) {
    if(m_endian != SYLAR_BYTE_ORDER) {
        value = byteswap(value);
    }
    write(&value, sizeof(value));
}


static uint32_t EncodeZigzag32(const int32_t& v) {
    if(v < 0) {
        return ((uint32_t)(-v)) * 2 - 1;    // 负数变成 
    } else {
        return v * 2;       
    }
}

static uint64_t EncodeZigzag64(const int64_t& v) {
    if(v < 0) {
        return ((uint64_t)(-v)) * 2 - 1;    // 负数变成 
    } else {
        return v * 2;       
    }
}

static int32_t DecodeZigzag32(const uint32_t& v) {
    return (v >> 1) ^ -(v & 1);
}

static int64_t DecodeZigzag64(const uint64_t& v) {
    return (v >> 1) ^ -(v & 1);
}

/// ? 为什么这里压缩不需要考虑字节序?  -> 解压是由我们写的代码决定的，不是由cpu
void ByteArray::writeInt32(int32_t value) {
    writeUint32(EncodeZigzag32(value));
}
void ByteArray::writeUint32(uint32_t value) {
    uint8_t tmp[5];     // 最多不超过5个字节
    uint8_t i = 0;
    while(value >= 0x80) {    // >=0x80,说明value的高7位的二进制肯定有1，则需要压缩
        tmp[i++] = (value & 0x7f) | 0x80;  // 1 + 低7位，构成一个记录
        value >> 7;     // 右
    }
    tmp[i++] = value;
    /// write（const char* str,int n), n 用来表示输出显示字符串中字符的个数。
    /// void ByteArray::read(char* buf, size_t size);
    write(tmp, i);
}
void ByteArray::writeInt64(int64_t value) {
    writeUint64(EncodeZigzag64(value));
}
void ByteArray::writeUint64(uint64_t value) {
    uint8_t tmp[10];        // 7*9=63 7*10=70， 因此需要10
    uint8_t i = 0;
    while(value >= 0x80) {
        tmp[i++] = (value & 0x7F) | 0x80;
        value >>= 7;
    }
    tmp[i++] = value;
    write(tmp, i);
}

void ByteArray::writeFloat(float value) {
    /// float和double与CPU无关。一般来说，编译器是按照IEEE标准解释的，即把float/double看作4/8个字符的数组进行解释。
    /// 因此，只要编译器是支持IEEE浮点标准的，就不需要考虑字节顺序
    uint32_t v;
    memcpy(&v, &value, sizeof(value));
    writeFuint32(v);        //
}
void ByteArray::writeDouble(double value) {
    uint64_t v;
    memcpy(&v, &value, sizeof(value));
    writeFuint64(v);
}

// 写入string类型，需要写长度,F16表示长度的位
void ByteArray::writeStringF16(const std::string& value) {
    writeFuint16(value.size());     // 先写长度
    write(value.c_str(), value.size()); // 再写数据
}
void ByteArray::writeStringF32(const std::string& value) {
    writeFuint32(value.size());
    write(value.c_str(), value.size());
}
void ByteArray::writeStringF64(const std::string& value) {
    writeFuint64(value.size());
    write(value.c_str(), value.size());
}
// 写入std::string类型的数据,用无符号Varint64作为长度类型(压缩的长度)
void ByteArray::writeStringVint(const std::string& value) {
    writeUint64(value.size());
    write(value.c_str(), value.size());
}

void ByteArray::writeStringWithoutLength(const std::string& value) {
    write(value.c_str(), value.size());
}


int8_t  ByteArray::readFint8() {
    /// 1字节直接读就行，没有字节序问题
    int8_t v;
    read(&v, sizeof(v));
    return v;
}
uint8_t ByteArray::readFuint8() {
    uint8_t v;
    read(&v, sizeof(v));
    return v;
}

#define XX(type) \
    type v; \
    read(&v, sizeof(v)); \
    if(m_endian == SYLAR_BYTE_ORDER) { \        // 判断字节序
        return v; \
    } else { \
        return byteswap(v); \
    }

int16_t ByteArray::readFint16() {
    XX(int16_t);
}
int16_t ByteArray::readFuint16() {
    XX(uint16_t);
}


int32_t ByteArray::readFint32() {
    XX(int32_t);
}
int32_t ByteArray::readFuint32() {
    XX(uint32_t);
}
int64_t ByteArray::readFint64() {
    XX(int64_t);
}
int64_t ByteArray::readFuint64(){
    XX(uint64_t);
}

#undef XX



int32_t     ByteArray::readInt32() {
    return DecodeZigzag32(readUint32());
}
uint32_t    ByteArray::readUint32() {
    uint32_t result = 0;
    for(int i = 0; i < 32; i += 7) {
        uint8_t b = readFuint8();   // 先读一个字节
        if (b < 0x80) {  // 没有最后一个字节
            result |= ((uint32_t)) << i;
            break;
        } else {
            result |= (((uint32_t)(b & 0x7f)) << i);    // 先读的在低位
        }
    }

    return result;
}
int64_t     ByteArray::readInt64() {
    return DecodeZigzag64(readUint64());
}
uint64_t    ByteArray::readUint64() {
    uint64_t result = 0;
    for(int i = 0; i < 64; i += 7) {
        uint8_t b = readFuint8();
        if(b < 0x80) {
            result |= ((uint64_t)b) << i;
            break;
        } else {
            result |= (((uint64_t)(b & 0x7f)) << i);
        }
    }
    return result;
}

float ByteArray::readFloat() {
    uint32_t v = readFuint32();     // 直接按32位读
    float value;
    memcpy(&value, &v, sizeof(v));  // 拷贝进去
    return value;
}
double ByteArray::readDouble() {
    uint64_t v = readFuint64();
    double value;
    memcpy(&value, &v, sizeof(v));
    return value;
}

std::string ByteArray::readStringF16() {
    uint16_t len = readFuint16();       // 先读长度

    std::string buff;
    buff.resize(len);
    read(&buff[0], len);       // 再读数据，注意这里给的是buff[0]
    return buff;
}
std::string ByteArray::readStringF32() {
    uint32_t len = readFuint32();
    std::string buff;
    buff.resize(len);
    read(&buff[0], len);
    return buff;
}
std::string ByteArray::readStringF64() {
    uint64_t len = readFuint64();
    std::string buff;
    buff.resize(len);
    read(&buff[0], len);
    return buff;
}
std::string ByteArray::readStringVint() {
    uint64_t len = readUint64();
    std::string buff;
    buff.resize(len);
    read(&buff[0], len);
    return buff;
}


// 内部操作
void ByteArray::clear() {
    m_position = m_size = 0;
    m_capacity = m_baseSize;

    // 释放内存
    Node* tmp = m_root->next;       // 只留一个起始结点
    while(tmp) {
        m_cur = tmp;
        tmp = tmp->next;
        delete m_cur;
    }

    m_cur = m_root;
    m_root->next = NULL;
}

// 注意buf是void，支持任何类型
void ByteArray::write(cosnt void* buf, size_t size) {
    if (size == 0) {
        return 0;
    }
    addCapacity(size);  // 确保容量足够

    size_t npos = m_position % m_baseSize;  // 当前结点的可用空间的起始位置
    size_t ncap = m_cur->size - npos;      // 当前结点的容量
    size_t bpos = 0;

    while(size > 0) {
        if (ncap >= size) {     //  当前结点容量就够
            memcpy(m_cur->ptr + npos, (const char*)buf + bpos, size);
            if (m_cur->size == (npos + size)) {     // 刚好将这个结点写满
                m_cur = m_cur->next;
            }

            m_position += size;
            bpos += size;
            size = 0;       // 实际上就是break;
        } else {
            memcpy(m_cur->ptr + npos, (const char*)buf + bpos, ncap);    // 先写完当前结点

            m_position += ncap;
            bpos += ncap;
            size -= ncap;   // 还要写入的数量
            m_cur = m_cur->next;
            n_cap = m_cur->size;
            npos = 0;
        }
    }

    // 更新m_size
    if (m_position > m_size) {
        m_size = m_position;
    }



}
void ByteArray::read(void* buf, size_t size) {
    if(size > getReadSize()) {  // 要读的比我们size还大
        throw std::out_of_range("not enough len");
    }

    size_t npos = m_position % m_baseSize;
    size_t ncap = m_cur->size - npos;
    size_t bpos = 0;

    while(size > 0) {
        if(ncap >= size) {
            memcpy((char*)buf + bpos, m_cur->ptr + npos, size);
            if(m_cur->size == (npos + size)) {
                m_cur = m_cur->next;
            }
            m_position += size;
            bpos += size;
            size = 0;
        } else {
            memcpy((char*)buf + bpos, m_cur->ptr + npos, ncap);
            m_position += ncap;
            bpos += ncap;
            size -= ncap;
            m_cur = m_cur->next;   // ???这里怎么是继续next呢? 
            ncap = m_cur->size;
            npos = 0;
        }
        // 读完的结点不就可以释放了???
}

void ByteArray::setPosition(size_t v) {

}

bool ByteArray::readFromFile(const std::string& name) {

}


void ByteArray::addCapacity(size_t size) {

}

}