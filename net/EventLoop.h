//
// Created by huangw on 22-10-1.
//

#ifndef SERVERLIB_EVENTLOOP_H
#define SERVERLIB_EVENTLOOP_H

#include <functional>
#include <vector>
#include <atomic>
#include <mutex>
#include <memory>

#include "../base/CurrentThread.h"
#include "../base/Timestamp.h"
#include "../base/noncopyable.h"
#include "Callback.h"
#include "TimerId.h"


class Channel;
class Poller;
class TimerQueue;

class EventLoop : noncopyable
{
public:
    using Functor = std::function<void()>;

    EventLoop();
    ~EventLoop();

    void loop();

    void quit();

    Timestamp pollReturnTime() const {return pollReturnTime_;}
    void printActiveChannels() const;

    void runInLoop(Functor cb);
    void queueInLoop(Functor cb);

    TimerId runAt(Timestamp time, TimerCallback cb);

    TimerId runAfter(double delay, TimerCallback cb);

    TimerId runEvery(double interval, TimerCallback cb);

    void cancel(TimerId timerId);

    size_t queueSize() const;

    void wakeup();
    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);
    bool hasChannel(Channel* channel);

    bool isInLoopThread() const {return threadId_ == CurrentThread::tid();}

private:
    void handleRead();
    void doPendingFuncTors();

    using ChannelLists = std::vector<Channel*>;

    std::atomic_bool looping_;
    std::atomic_bool quit_;

    const pid_t threadId_;
    Timestamp pollReturnTime_;
    std::unique_ptr<Poller> poller_;
    std::unique_ptr<TimerQueue> timerQueue_;

    int wakeupFd_;  //使用此fd来使poller跳出阻塞，执行下面的任务
    std::unique_ptr<Channel> wakeupChannel_;

    ChannelLists activeChannels_;


    std::atomic_bool callingPendingFunctors_;//标识，是否有需要loop执行的回调函数
    std::vector<Functor> pendingFunctors_; //存储loop需要执行的所有回调函数
    mutable std::mutex mutex_;

};


#endif //SERVERLIB_EVENTLOOP_H
