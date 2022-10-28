//
// Created by huangw on 22-10-26.
//

#ifndef SERVERLIB_TIMERQUEUE_H
#define SERVERLIB_TIMERQUEUE_H

#include <set>
#include <vector>

#include "Callback.h"
#include "Channel.h"

class EventLoop;
class Timer;
class TimerId;

class TimerQueue : noncopyable
{
public:
    TimerQueue(EventLoop* loop);
    ~TimerQueue();

    TimerId addTimer(TimerCallback cb, Timestamp when, double interval);

    void cancel(TimerId timerId);

private:
    using Entry = std::pair<Timestamp,Timer*>;
    using TimerList = std::set<Entry>;           //按时间排序的定时器列表

    using ActiveTimer = std::pair<Timer*,int64_t>;
    using ActiveTimerSet = std::set<ActiveTimer>;  //按定时器地址排序的定时器列表

    void addTimerInLoop(Timer* timer);
    void cancelInLoop(TimerId timerId);

    void handleRead();
    std::vector<Entry> getExpired(Timestamp now);

    void reset(const std::vector<Entry>& expired, Timestamp now);

    bool insert(Timer* timer);


    EventLoop* loop_;
    const int timerfd_;
    Channel timerfdChannel_;
    TimerList  timers_;

    ActiveTimerSet activeTimers_;
    bool callingExpiredTimers_;
    ActiveTimerSet cancelingTimers_;
};


#endif //SERVERLIB_TIMERQUEUE_H
