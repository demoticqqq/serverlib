//
// Created by huangw on 22-10-4.
//

#include "EventLoopThread.h"
#include "EventLoop.h"

EventLoopThread::EventLoopThread(const ThreadInitCallback &cb ,const std::string &name)
        : callback_(cb)
        , thread_(std::bind(&EventLoopThread::threadFunc,this),name)
        , loop_(nullptr)
        , exiting_(false)
        , mutex_()
        , cond_()
{
}

EventLoopThread::~EventLoopThread()
{
    exiting_ = true;
    if(loop_ != nullptr)
    {
        loop_->quit();
        thread_.join();
    }
}

EventLoop *EventLoopThread::startLoop()
{
    thread_.start();
    EventLoop* loop = nullptr;
    {
        std::unique_lock<std::mutex>lk(mutex_);
        cond_.wait(lk,[&](){
            return loop_ != nullptr;
        });
        loop_ = loop;
    }
    return loop;
}

void EventLoopThread::threadFunc()
{
    EventLoop loop;
    if(callback_)
    {
        callback_(&loop);
    }
    {
        std::unique_lock<std::mutex>lk(mutex_);
        loop_ = &loop;
        cond_.notify_one();
    }
    loop_->loop();
    std::unique_lock<std::mutex>lk(mutex_);
    loop_ = nullptr;
}