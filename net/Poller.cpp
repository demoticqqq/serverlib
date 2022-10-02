//
// Created by huangw on 22-10-1.
//

#include "Poller.h"
#include "Channel.h"

Poller::Poller(EventLoop *loop)
    :ownerLoop_(loop)
{
}

Poller::~Poller() = default;

bool Poller::hasChannel(Channel *channel) const
{
    auto it = channels_.find(channel->fd());
    return it != channels_.end() && it->second == channel;
 }