//
// Created by huangw on 22-10-4.
//

#include "EventLoopThreadPool.h"
#include "EventLoopThread.h"

EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseloop, const std::string &name)
    :baseLoop_(baseloop)
    ,name_(name)
    ,started_(false)
    ,numThreads_(0)
    ,next_(0)
{
}

EventLoopThreadPool::~EventLoopThreadPool()
{

}

void EventLoopThreadPool::start(const EventLoopThreadPool::ThreadInitCallback &cb)
{
    for(int i = 0; i < numThreads_; i++)
    {
        char buf[name_.size()+32];
        snprintf(buf,sizeof buf,"%s%d",name_.c_str(),i);
        EventLoopThread *t = new EventLoopThread(cb,buf);
        threads_.emplace_back(std::unique_ptr<EventLoopThread>(t));
        loops_.emplace_back(t->startLoop());
    }
    if(numThreads_ == 0 && cb)
    {
        cb(baseLoop_);
    }
}

EventLoop *EventLoopThreadPool::getNextLoop()
{
    /*如果只设置一个线程 也就是只有一个mainReactor 无subReactor
    那么轮询只有一个线程 getNextLoop()每次都返回当前的baseLoop_*/
    EventLoop* loop = baseLoop_;
    if(!loops_.empty())
    {
        loop = loops_[next_];
        next_++;
        if(static_cast<size_t>(next_) >= loops_.size())
        {
            next_=0;
        }
    }
    return loop;
}

EventLoop *EventLoopThreadPool::getLoopForHash(size_t hashCode)
{
    EventLoop* loop = baseLoop_;

    if (!loops_.empty())
    {
        loop = loops_[hashCode % loops_.size()];
    }
    return loop;
}

std::vector<EventLoop *> EventLoopThreadPool::getAllLoops()
{
    if(loops_.empty())
    {
        return std::vector<EventLoop*>(1,baseLoop_);
    }
    return loops_;
}