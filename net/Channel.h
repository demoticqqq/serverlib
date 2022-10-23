//
// Created by huangw on 22-10-1.
//

#ifndef SERVERLIB_CHANNEL_H
#define SERVERLIB_CHANNEL_H

#include <functional>
#include <memory>
#include "../base/noncopyable.h"
#include "../base/Timestamp.h"


class EventLoop;

class Channel : noncopyable
{
public:
    using EventCallback = std::function<void()>;
    using ReadEventCallback = std::function<void(Timestamp)>;

    Channel(EventLoop *loop, int fd);
    ~Channel();

    void tie(const std::shared_ptr<void>&);

    void handleEvent(Timestamp receiveTime);
    void setReadCallback(ReadEventCallback cb) {readCallback_=std::move(cb);}
    void setWriteCallback(EventCallback cb) {writeCallback_=std::move(cb);}
    void setCloseCallback(EventCallback cb) {closeCallback_=std::move(cb);}
    void setErrorCallback(EventCallback cb) {errorCallback_=std::move(cb);}

    int fd() const { return fd_;}
    int events() const { return events_;}
    void set_revents(int revt) { revents_ = revt;}
    //设置fd的监听事件，等于epoll_ctl_add/delete
    void enableReading() {events_ |= kReadEvent; update();}
    void enableWriting() {events_ |= kWriteEvent; update();}
    void disableReading() {events_ &= ~kReadEvent; update();}
    void disableWriting() {events_ &= ~kWriteEvent; update();}
    void disableAll() {events_ = kNoneEvent; update();}

    bool isNoneEvent() const {return events_ == kNoneEvent;}
    bool isWriting() const {return events_ & kWriteEvent;}
    bool isReading() const {return events_ & kReadEvent;}

    int index() {return index_;}
    void setIndex(int idx) {index_ = idx;}

    EventLoop *ownerLoop() {return loop_;}
    void remove();

private:
    void update();
    void handleEventWithGuard(Timestamp receiveTime);

    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    EventLoop* loop_;
    const int fd_;
    int events_;
    int revents_;
    int index_;   //Poller使用此参数

    std::weak_ptr<void> tie_;
    bool tied_;
    ReadEventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;
};


#endif //SERVERLIB_CHANNEL_H
