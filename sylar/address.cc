#include <address.h>
#include <sstream>

namespace sylar {


int Address::getFamily() const {
    return getAddr()->sa_family;
}

std::string Address::toString() {
    std::stringstream ss;
    insert(ss);  // 每个地址提供insert函数，将信息插入string流中
    return ss.str();
}

bool Address::operator<(const Address& rhs) const {
    socklen_t minlen = std::min(getAddrLen() ,rhs.getAddrLen);
    int result = memcmp(getAddr(), rhs.getAddr(), minlen);      // 比较内存块

    if (result < 0) {
        return true;
    } else if (result > 0) {
        return false;
    } else if (getAddrLen() < rhs.getAddrLen) {
        return true;
    }
    return false;
}
bool Address::operator==(const Address& rhs) const {
    return getAddrLen() == rhs.getAddrLen &&
           memcmp(getAddr(), rhs.getAddr(), getAddrLen()) == 0;
}
bool Address::operator!=(const Address& rhs) const {
    return !(*this == rhs);
}

IPv4Address::IPv4Address(uint32_t address, uint32_t port) {

}

const sockaddr* IPv4Address::getAddr() const {

}
socklen_t Ipv4Address::getAddrLen() const {

}
std::ostream& Ipv4Address::insert(std::ostream&os ) const {

}

IPAddress::ptr Ipv4Address::broadcastAddress(uint32_t prefix_len) {

}
IPAddress::ptr Ipv4Address::networdAddress(uint32_t prefix_len) {

}
IPAddress::ptr Ipv4Address::subnetMask(uint32_t prefix_len) {

}

uint32_t Ipv4Address::getPort() const {

}
void Ipv4Address::setPort() {

}

IPv6Address::IPv6Address(uint32_t address, uint32_t port) {

}

const sockaddr* IPv6ddress::getAddr() const {

}
socklen_t Ipv6Address::getAddrLen() const {

}
std::ostream& Ipv6Address::insert(std::ostream&os ) const {

}

IPAddress::ptr Ipv6Address::broadcastAddress(uint32_t prefix_len) {

}
IPAddress::ptr Ipv6Address::networdAddress(uint32_t prefix_len) {

}
IPAddress::ptr Ipv6Address::subnetMask(uint32_t prefix_len) {

}

uint32_t Ipv6Address::getPort() const {

}
void Ipv6Address::setPort() {

}

UnknownAddress::UnknownAddress(const sockaddr& addr){

}

UnknownAddress::UnknownAddress() {

}

const sockaddr* UnknownAddress::getAddr() const {

}
socklen_t UnknownAddress::getAddrLen() const {

}
std::ostream& UnknownAddress::insert(std::ostream& os) const{
   
}
} // syalr
