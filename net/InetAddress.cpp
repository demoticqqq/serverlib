//
// Created by huangw on 22-10-4.
//
#include <netdb.h>
#include <netinet/in.h>
#include <memory>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "InetAddress.h"
#include "../base/Logger.h"

InetAddress::InetAddress(uint16_t port, bool loopbackOnly, bool ipv6)
{
    if (ipv6)
    {
        memset(&addr6_,0, sizeof addr6_);
        addr6_.sin6_family = AF_INET6;
        in6_addr ip = loopbackOnly ? in6addr_loopback : in6addr_any;
        addr6_.sin6_addr = ip;
        addr6_.sin6_port = htobe16(port);
    }
    else
    {
        memset(&addr_,0, sizeof addr_);
        addr_.sin_family = AF_INET;
        in_addr_t ip = loopbackOnly ? INADDR_LOOPBACK : INADDR_ANY;
        addr_.sin_addr.s_addr = htobe32(ip);
        addr_.sin_port = htobe16(port);
    }
}

InetAddress::InetAddress(std::string ip, uint16_t port, bool ipv6)
{
    if (ipv6 || strchr(ip.c_str(), ':'))
    {
        memset(&addr6_, 0,sizeof addr6_);
        addr6_.sin6_family = AF_INET6;
        addr6_.sin6_port = htobe16(port);
        if (::inet_pton(AF_INET6, ip.c_str(), &addr6_.sin6_addr) <= 0)
        {
            LOG_ERROR("func=%p : sockets::fromIpPort\n",__FUNCTION__ );
        }
    }
    else
    {
        memset(&addr_, 0,sizeof addr_);
        addr_.sin_family = AF_INET;
        addr_.sin_port = htobe16(port);
        if (::inet_pton(AF_INET, ip.c_str(), &addr_.sin_addr) <= 0)
        {
            LOG_ERROR("func=%p : sockets::fromIpPort\n",__FUNCTION__ );
        }
    }
}

std::string InetAddress::toIp() const
{
    char buf[64]={0};
    const sockaddr* addr = static_cast<const sockaddr*>(getSockAddr());
    if(addr->sa_family==AF_INET)
    {
        ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof buf);
    }
    else
    {
        ::inet_ntop(AF_INET6,&addr6_.sin6_port,buf,sizeof buf);
    }
}

std::string InetAddress::toIpPort() const
{
    char buf[64] = {0};
    const sockaddr* addr = static_cast<const sockaddr*>(getSockAddr());
    if(addr->sa_family==AF_INET)
    {
        ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof buf);
        size_t end = ::strlen(buf);
        uint16_t port = ::ntohs(addr_.sin_port);
        sprintf(buf+end, ":%u", port);
    }
    else
    {
        ::inet_ntop(AF_INET6,&addr6_.sin6_port,buf,sizeof buf);
        size_t end = ::strlen(buf);
        uint16_t port = ::ntohs(addr6_.sin6_port);
        sprintf(buf+end, ":%u", port);
    }

    return buf;
}

uint16_t InetAddress::toPort() const
{
    return ::ntohs(portNetEndian());
}

uint32_t InetAddress::ipv4NetEndian() const
{
    return addr_.sin_addr.s_addr;
}

static __thread char t_resolveBuffer[64 * 1024];

bool InetAddress::resolve(std::string hostname, InetAddress* out)
{
    struct hostent hent;
    struct hostent* he = NULL;
    int herrno = 0;
    memset(&hent, 0,sizeof(hent));
    int ret = gethostbyname_r(hostname.c_str(), &hent, t_resolveBuffer, sizeof t_resolveBuffer, &he, &herrno);
    if (ret == 0 && he != NULL)
    {
        out->addr_.sin_addr = *reinterpret_cast<struct in_addr*>(he->h_addr);
        return true;
    }
    else
    {
        if (ret)
        {
            LOG_ERROR("func= %p : InetAddress::resolve\n");
        }
        return false;
    }
}

void InetAddress::setScopeId(uint32_t scope_id)
{
    if (family() == AF_INET6)
    {
        addr6_.sin6_scope_id = scope_id;
    }
}