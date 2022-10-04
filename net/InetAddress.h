//
// Created by huangw on 22-10-4.
//

#ifndef SERVERLIB_INETADDRESS_H
#define SERVERLIB_INETADDRESS_H
#include <netinet/in.h>
#include <boost/implicit_cast.hpp>
#include <string>
class InetAddress
{
public:
    explicit InetAddress(uint16_t port = 0,bool loopbackOnly = false,bool ipv6 = false); //默认ipv4，开启环回地址

    InetAddress(std::string ip,uint16_t port,bool ipv6 = false);//ip+port构造，默认ipv4
    explicit InetAddress(const sockaddr_in addr)
        :addr_(addr)
    {
    }
    explicit InetAddress(const sockaddr_in6 addr)
        :addr6_(addr)
    {
    }

    sa_family_t family() const {return addr_.sin_family;}
    std::string toIp() const;
    std::string toIpPort() const;
    uint16_t toPort() const;

    const sockaddr* getSockAddr() const {return static_cast<const sockaddr*>(static_cast<const void *>(&addr6_));}
    void setSockAddrInet6(const sockaddr_in6& addr6) {addr6_ = addr6;}
    void setSockAddrInet4(const sockaddr_in& addr) {addr_ = addr;}

    uint32_t ipv4NetEndian() const;
    uint16_t portNetEndian() const {return addr_.sin_port;}
    // resolve hostname to IP address, not changing port or sin_family
    static bool resolve(std::string hostname,InetAddress* result);

    // set IPv6 ScopeID
    void setScopeId(uint32_t scoped_id);

private:
    union
    {
        sockaddr_in addr_;
        sockaddr_in6 addr6_;
    };
};


#endif //SERVERLIB_INETADDRESS_H
