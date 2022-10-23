//
// Created by huangw on 22-10-22.
//
#include <stdlib.h>

#include "Poller.h"
#include "EpollPoller.h"

Poller *Poller::newDefaultPoller(EventLoop *loop)
{
    if (::getenv("SERVERLIB_USE_POLL"))
    {
        return nullptr; // 生成poll的实例
    }
    else
    {
        return new EpollPoller(loop); // 生成epoll的实例
    }
}