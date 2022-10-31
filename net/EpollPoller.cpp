//
// Created by huangw on 22-10-1.
//
#include <string.h>
#include <stdio.h>
#include "EpollPoller.h"
#include "../base/Logger.h"
#include "Channel.h"

const int kNew = -1;
const int kAdded = 1;
const int kDeleted = 2;

EpollPoller::EpollPoller(EventLoop *loop)
    : Poller(loop)
    , epollfd_(epoll_create(EPOLL_CLOEXEC))
    , events_(kInitEventListSize)
{
    if(epollfd_ < 0)
    {
        LOG_FATAL("EpollPoller: epoll_create error:%d\n",errno);
    }
}

EpollPoller::~EpollPoller()
{
    close(epollfd_);
}

Timestamp EpollPoller::poll(int timeoutMs, Poller::ChannelList *activeChannels)
{
    LOG_INFO("func=%s : fd total count %lu\n",__FUNCTION__ ,channels_.size());

    int numEvents = epoll_wait(epollfd_,events_.data(),static_cast<int>(events_.size()),timeoutMs);
    int savedErrno = errno;
    Timestamp now(Timestamp::now());
    if(numEvents > 0)
    {
        LOG_INFO("func=%s : %d events happend\n", __FUNCTION__ ,numEvents);
        fillActiveChannels(numEvents,activeChannels);
        if(numEvents == events_.size())
        {
            events_.resize(events_.size() * 2);
        }
    }
    else if (numEvents == 0)
    {
        LOG_DEBUG("func=%s : timeout!\n", __FUNCTION__);
    }
    else
    {
        if(savedErrno != EINTR)
        {
            errno = savedErrno;
            LOG_ERROR("EPollPoller::poll() error!\n");
        }
    }
    return now;
}

void EpollPoller::fillActiveChannels(int numEvents, Poller::ChannelList *activeChannels) const
{
    for(int i = 0; i < numEvents; i++)
    {
        Channel *channel = static_cast<Channel*>(events_[i].data.ptr);
        channel->set_revents(events_[i].events);
        activeChannels->emplace_back(channel);
    }
}

void EpollPoller::updateChannel(Channel *channel)
{
    int index = channel->index();
    LOG_INFO("func=%s => fd=%ld events=%ld index=%d\n", __FUNCTION__, channel->fd(), channel->events(), index);

    if(index == kNew || index == kDeleted)
    {
        //新的fd加入
        int fd = channel->fd();
        if(index == kNew)
        {
            channels_[fd] = channel;
        }
        else  //之前删除的fd，但仍在channeMap里
        {
        }
        channel->setIndex(kAdded);
        update(EPOLL_CTL_ADD,channel);
    }
    else
    {
        int fd = channel->fd();
        if(channel->isNoneEvent())
        {
            update(EPOLL_CTL_DEL,channel);    //仅从epoll下树，并未删除channel
            channel->setIndex(kDeleted);
        }
        else
        {
            update(EPOLL_CTL_MOD,channel);
        }
    }
}

void EpollPoller::removeChannel(Channel *channel)
{
    int fd = channel->fd();
    LOG_DEBUG("func=%s => fd=%d\n", __FUNCTION__, fd);
    int index = channel->index();
    channels_.erase(fd);

    if(index == kAdded) //kAdded 说明还未从epoll上下树，则执行下树操作
    {
        update(EPOLL_CTL_DEL,channel);
    }
    channel->setIndex(kNew);
}

void EpollPoller::update(int operation, Channel *channel)
{
    epoll_event event;
    memset(&event,0,sizeof event);
    int fd = channel->fd();
    event.events = channel->events();
    //event.data.fd = fd;
    event.data.ptr = channel;

    if(epoll_ctl(epollfd_,operation,fd,&event) < 0)
    {
        if(operation == EPOLL_CTL_DEL)
        {
            LOG_ERROR("fd = %d : epoll_ctl del error:%d\n",fd,errno);
        }
        else
        {
            LOG_FATAL("fd = %d : epoll_ctl add/mod error:%d\n", fd,errno);
        }
    }
}