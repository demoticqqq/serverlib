//
// Created by huangw on 22-10-1.
//

#ifndef SERVERLIB_EPOLLPOLLER_H
#define SERVERLIB_EPOLLPOLLER_H

#include <vector>
#include <sys/epoll.h>

#include "Poller.h"
#include "../base/Timestamp.h"

class EpollPoller : public Poller
{
public:
    EpollPoller(EventLoop *loop);
    ~EpollPoller() override;

    Timestamp poll(int timeoutMs,ChannelList *activeChannels);
    void updateChannel(Channel *channel) override;
    void removeChannel(Channel *channel) override;

private:
    static const int kInitEventListSize = 16;

    void fillActiveChannels(int numEvents,ChannelList *activeChannels) const;
    void update(int operation,Channel *channel);

    using EventList = std::vector<epoll_event>;

    int epollfd_;
    EventList events_;

};


#endif //SERVERLIB_EPOLLPOLLER_H
