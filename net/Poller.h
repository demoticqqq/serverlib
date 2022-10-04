//
// Created by huangw on 22-10-1.
//

#ifndef SERVERLIB_POLLER_H
#define SERVERLIB_POLLER_H

#include "EventLoop.h"
#include <unordered_map>

class EventLoop;

class Poller : noncopyable
{
public:
    using ChannelList = std::vector<Channel *>;

    Poller(EventLoop* loop);
    virtual ~Poller();

    virtual Timestamp poll(int timeoutMs,ChannelList* activeChannels) = 0;
    virtual void updateChannel(Channel* channel) = 0;
    virtual void removeChannel(Channel* channel) = 0;
    bool hasChannel(Channel* channel) const;

    static Poller* newDefaulPoller(EventLoop* loop);

protected:
    using ChannelMap = std::unordered_map<int,Channel*>;
    ChannelMap channels_;

private:
    EventLoop* ownerLoop_;
};


#endif //SERVERLIB_POLLER_H
