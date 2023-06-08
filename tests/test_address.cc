#include "../sylar/address.h"
#include "../sylar/log.h"

sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void test() {
/// 取出某个host相关的地址

    std::vector<sylar::Address::ptr> addrs;

    bool v = sylar::Address::Lookup(addrs, "www.baidu.com", AF_INET);
    // bool v = sylar::Address::Lookup(addrs, "www.baidu.com:80", AF_INET);
    // bool v = sylar::Address::Lookup(addrs, "www.baidu.com:http", AF_INET);
    if(!v) {
        SYLAR_LOG_ERROR(g_logger) << "lookup fail";
        return;
    }

    for(size_t i = 0; i < addrs.size(); ++i) {
        SYLAR_LOG_INFO(g_logger) << i << " - " << addrs[i]->toString();
    }
    
}

void test_iface() {
    /// 获得网卡信息
    std::multimap<std::string, std::pair<sylar::Address::ptr, uint32_t> > results;

    // bool v = sylar::Address::GetInterfaceAddresses(results,AF_INET6);
    // bool v = sylar::Address::GetInterfaceAddresses(results);
    bool v = sylar::Address::GetInterfaceAddresses(results, AF_UNSPEC);
    if(!v) {
        SYLAR_LOG_ERROR(g_logger) << "GetInterfaceAddresses fail";
        return;
    }

    for(auto& i: results) {
        SYLAR_LOG_INFO(g_logger) << i.first << " - " << i.second.first->toString() << " - "
            << i.second.second;
    }
}

void test_ipv4() {
    // auto addr = sylar::IPAddress::Create("www.sylar.top"); //域名不行
    // auto addr = sylar::IPAddress::Create("127.0.0.8");
    auto addr = sylar::IPv4Address::Create("127.0.0.8");
    if(addr) {
        SYLAR_LOG_INFO(g_logger) << addr->toString();
    }
}
int main() {
    // test();
    // test_iface();
    test_ipv4();
    return 0;
}