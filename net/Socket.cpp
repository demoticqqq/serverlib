//
// Created by huangw on 22-10-4.
//
#include "Socket.h"
#include <cstring>
#include <netinet/tcp.h>
#include "../base/Logger.h"

void Socket::bindAddress(const InetAddress &localaddr)
{
    const sockaddr* addr = static_cast<const sockaddr*>(localaddr.getSockAddr());
    int ret = ::bind(sockfd_, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
    if (ret < 0)
    {
        LOG_FATAL("bind sockfd:%d fail\n", sockfd_);
    }
}

void Socket::listen()
{
    int ret = ::listen(sockfd_, SOMAXCONN);
    if (ret < 0)
    {
        LOG_FATAL("listen sockfd:%d fail\n", sockfd_);
    }
}

int Socket::accept(InetAddress *peeraddr)
{
    sockaddr_in6 addr;
    socklen_t len = sizeof(addr);
    memset(&addr, 0, sizeof(addr));
    int connfd = ::accept4(sockfd_, (sockaddr *)&addr, &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (connfd >= 0)
    {
        peeraddr->setSockAddrInet6(addr);
    }
    else
    {
        int savedErrno = errno;
        LOG_ERROR("Socket::accept\n");
        switch (savedErrno)
        {
            case EAGAIN:
            case ECONNABORTED:
            case EINTR:
            case EPROTO: // ???
            case EPERM:
            case EMFILE: // per-process lmit of open file desctiptor ???
                // expected errors
                errno = savedErrno;
                break;
            case EBADF:
            case EFAULT:
            case EINVAL:
            case ENFILE:
            case ENOBUFS:
            case ENOMEM:
            case ENOTSOCK:
            case EOPNOTSUPP:
                // unexpected errors
                LOG_FATAL("unexpected error of ::accept %d\n",savedErrno);
                break;
            default:
                LOG_FATAL("unknown error of ::accept %d\n",savedErrno);
                break;
        }
    }
    return connfd;
}

void Socket::shutdownWrite()
{
    if (::shutdown(sockfd_, SHUT_WR) < 0)
    {
        LOG_ERROR("sockets::shutdownWrite\n");
    }
}

void Socket::setTcpNoDelay(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY,
                 &optval, static_cast<socklen_t>(sizeof optval));
}

void Socket::setReuseAddr(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR,
                 &optval, static_cast<socklen_t>(sizeof optval));
}

void Socket::setReusePort(bool on)
{
#ifdef SO_REUSEPORT
    int optval = on ? 1 : 0;
    int ret = ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT,
                           &optval, static_cast<socklen_t>(sizeof optval));
    if (ret < 0 && on)
    {
        LOG_ERROR("sockfd:%d set SO_REUSEPORT failed\n",sockfd_ );
    }
#else
    if (on)
    {
        LOG_ERROR("SO_REUSEPORT is not supported\n");
    }
#endif
}

void Socket::setKeepAlive(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE,
                 &optval, static_cast<socklen_t>(sizeof optval));
}