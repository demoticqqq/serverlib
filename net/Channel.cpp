//
// Created by huangw on 22-10-1.
//
#include <sys/epoll.h>

#include "Channel.h"
#include "EventLoop.h"
#include "../base/Logger.h"

const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::kWriteEvent = EPOLLOUT;
const int Channel::kNoneEvent = 0;

Channel::Channel(EventLoop *loop, int fd)
    :loop_(loop)
    ,fd_(fd)
    ,events_(0)
    ,revents_(0)
    ,index_(-1)
    ,tied_(false)
{
}

Channel::~Channel()
{
}
/**channel的tie方法什么时候调用?  TcpConnection => channel
/**
 * TcpConnection中注册了Chnanel对应的回调函数，传入的回调函数均为TcpConnection
 * 对象的成员方法，因此可以说明一点就是：Channel的结束一定早于TcpConnection对象！
 * 此处用tie去解决TcoConnection和Channel的生命周期时长问题，从而保证了Channel对
 * 象能够在TcpConnection销毁前销毁。
 **/
 void Channel::tie(const std::shared_ptr<void> &obj)
{
    tie_ = obj;
    tied_ = true;
}

void Channel::update()
{
    loop_->updateChannel(this);
}

void Channel::remove()
{
    loop_->removeChannel(this);
}

void Channel::handleEvent(Timestamp receiveTime)
{
    if(tied_)
    {

        std::shared_ptr<void> guard = tie_.lock();
        if(guard)
        {
            handleEventWithGuard(receiveTime);
        }
    }
    else
    {
        handleEventWithGuard(receiveTime);
    }
}

void Channel::handleEventWithGuard(Timestamp receiveTime)
{
    LOG_INFO("channel handleEvent revents:%d\n", revents_);
    //关闭连接
    if( (revents_ & EPOLLHUP) && !(revents_ & EPOLLIN) )
    {
        if(closeCallback_)
        {
            closeCallback_();
        }
    }
    //错误
    if(revents_ & EPOLLERR)
    {
        if(errorCallback_)
        {
            errorCallback_();
        }
    }
    //读
    if(revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP))
    {
        if(readCallback_)
        {
            readCallback_(receiveTime);
        }
    }
    if(revents_ & EPOLLOUT)
    {
        if(writeCallback_)
        {
            writeCallback_();
        }
    }
}