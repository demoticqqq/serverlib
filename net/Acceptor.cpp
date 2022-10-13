//
// Created by huangw on 22-10-13.
//
#include <unistd.h>
#include <sys/socket.h>
#include <fcntl.h>

#include "Acceptor.h"
#include "../base/Logger.h"
#include "InetAddress.h"

static int createNonblockingSockfd(sa_family_t family)
{
    int sockfd = ::socket(family,SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC,IPPROTO_TCP);
    if(sockfd < 0)
    {
        LOG_FATAL("%s:%s:%d listen socket create err:%d\n", __FILE__, __FUNCTION__, __LINE__, errno);
    }
    return sockfd;
}

Acceptor::Acceptor(EventLoop *loop, const InetAddress &listenAddr, bool reuseport)
    : loop_(loop)
    , acceptSocket_(createNonblockingSockfd(listenAddr.family()))
    , acceptChannel_(loop,acceptSocket_.fd())
    , listening_(false)
    , idleFd(open("/dev/null",O_RDONLY | O_CLOEXEC))
{
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.setReusePort(reuseport);
    acceptSocket_.bindAddress(listenAddr);
    acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor()
{
    acceptChannel_.disableAll();
    acceptChannel_.remove();
    close(idleFd);
}

void Acceptor::listen()
{
    listening_ = true;
    acceptSocket_.listen();          //事件监听
    acceptChannel_.enableReading();     //通过channel注册到Poller上

}

void Acceptor::handleRead()
{
    InetAddress peerAddr;
    int connfd = acceptSocket_.accept(&peerAddr);
    if(connfd >= 0)
    {
        if(newConnectCallback_)
        {
            newConnectCallback_(connfd,peerAddr);
        }
        else
        {
            close(connfd);
        }
    }
    else
    {
        LOG_ERROR("%s:%s:%d accept err:%d\n", __FILE__, __FUNCTION__, __LINE__, errno);
        if(errno == EMFILE)   //文件描述符达到上限
        {
            close(idleFd);
            idleFd = accept(acceptSocket_.fd(), nullptr, nullptr);  //用提前预留的fd来优雅地关闭连接
            close(idleFd);
            idleFd = open("/dev/null",O_RDONLY | O_CLOEXEC);
        }
    }
}