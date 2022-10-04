//
// Created by huangw on 22-10-4.
//

#ifndef SERVERLIB_SOCKET_H
#define SERVERLIB_SOCKET_H

#include "../base/noncopyable.h"
#include "InetAddress.h"

class Socket : noncopyable
{
public:
    explicit Socket(int sockfd)
        :sockfd_(sockfd)
    {
    }
    ~Socket();

    int fd() const {return sockfd_;}
    void bindAddress(const InetAddress &localaddr);
    void listen();
    int accept(InetAddress *peeraddr);//保存对端地址

    void shutdownWrite();

    void setTcpNoDelay(bool on);
    void setReuseAddr(bool on);
    void setReusePort(bool on);
    void setKeepAlive(bool on);

private:
    int sockfd_;
};


#endif //SERVERLIB_SOCKET_H
