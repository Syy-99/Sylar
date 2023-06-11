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



void ByteArray::writeInt32(const int32_t value);
void ByteArray::writeUint32(const uint32_t value);
void ByteArray::writeInt64(const int64_t value);
void ByteArray::writeUint64(const uint64_t value);

void ByteArray::writeFloat(float value);
void ByteArray::writeDouble(double value)

// 写入string类型，需要写长度,F16表示长度的位
void ByteArray::writeStringF16(const std::string& value);
void ByteArray::writeStringF32(const std::string& value);
void ByteArray::writeStringF64(const std::string& value);
// 写入std::string类型的数据,用无符号Varint64作为长度类型(压缩的长度)
void ByteArray::writeStringVint(const std::string& value);

void ByteArray::writeStringWithoutLength(const std::string& value);


int8_t  ByteArray::readFint8();
uint8_t ByteArray::readFuint16();
int16_t ByteArray::readFint16();
int16_t ByteArray::readFuint16();
int32_t ByteArray::readFint32();
int32_t ByteArray::readFuint32();
int64_t ByteArray::readFint64();
int64_t ByteArray::readFuint64();

int32_t     ByteArray::readInt32();
uint32_t    ByteArray::readUint32();
int64_t     ByteArray::readInt64();
uint64_t    ByteArray::readUint64();

float ByteArray::readFloat();
double ByteArray::readDouble();

std::string ByteArray::readStringF16();
std::string ByteArray::readStringF32();
std::string ByteArray::readStringF64();
std::string ByteArray::readStringVint();


// 内部操作
void ByteArray::clear();

void ByteArray::write(cosnt void* buf, size_t size);
void ByteArray::read(char* buf, size_t size);

void ByteArray::setPosition(size_t v);

bool ByteArray::readFromFile(const std::string& name);


void ByteArray::addCapacity(size_t size);

}