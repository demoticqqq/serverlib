//
// Created by huangw on 22-10-4.
//

#ifndef SERVERLIB_EVENTLOOPTHREAD_H
#define SERVERLIB_EVENTLOOPTHREAD_H

#include <functional>
#include <mutex>
#include <condition_variable>
#include "../base/Thread.h"

class EventLoop;

class EventLoopThread : noncopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    EventLoopThread(const ThreadInitCallback &cb = ThreadInitCallback(),const std::string &name = std::string());
    ~EventLoopThread();

    EventLoop* startLoop();


private:
    void threadFunc();

    EventLoop* loop_;
    bool exiting_;
    Thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;
    ThreadInitCallback callback_;

};


#endif //SERVERLIB_EVENTLOOPTHREAD_H
