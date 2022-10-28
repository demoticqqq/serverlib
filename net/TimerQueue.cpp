//
// Created by huangw on 22-10-26.
//

#include "TimerQueue.h"
#include "../base/Logger.h"
#include "EventLoop.h"
#include "Timer.h"
#include "TimerId.h"

#include <sys/timerfd.h>
#include <assert.h>
#include <string.h>

int createTimerfd()
{
    int timerfd = timerfd_create(CLOCK_MONOTONIC,TFD_NONBLOCK | TFD_CLOEXEC);
    if(timerfd < 0)
    {
        LOG_ERROR("Failed in timerfd_create");
    }
    return timerfd;
}

timespec howMuchTimeFromNow(Timestamp when)
{
    int64_t microseconds = when.microSecondsSinceEpoch() - Timestamp::now().microSecondsSinceEpoch();
    if(microseconds < 100)
    {
        microseconds = 100;
    }
    timespec ts;
    ts.tv_sec = static_cast<time_t>(microseconds/Timestamp::kMicroSecondsPerSecond);
    ts.tv_nsec = static_cast<long>((microseconds % Timestamp::kMicroSecondsPerSecond) * 1000);
    return ts;
}

void readTimerfd(int timerfd,Timestamp now)
{
    uint64_t howmany;
    ssize_t n = read(timerfd,&howmany,sizeof howmany);
    if(n != sizeof howmany)
    {
        LOG_ERROR("TimerQueue::handleRead() reads %ld bytes instead of 8");
    }
}

void resetTimerfd(int timerfd,Timestamp expiration)
{
    itimerspec newValue;
    itimerspec oldValue;
    memset(&newValue,0,sizeof newValue);
    memset(&oldValue,0,sizeof oldValue);
    newValue.it_value = howMuchTimeFromNow(expiration);
    int ret = timerfd_settime(timerfd,0,&newValue,&oldValue);
    if(ret)
    {
        LOG_ERROR("timerfd_settime() error...");
    }
}

TimerQueue::TimerQueue(EventLoop *loop)
    : loop_(loop)
    , timerfd_(createTimerfd())
    , timerfdChannel_(loop,timerfd_)
    , timers_()
    , callingExpiredTimers_(false)
{
    timerfdChannel_.setReadCallback(std::bind(&TimerQueue::handleRead,this));
    timerfdChannel_.enableReading();
}
TimerQueue::~TimerQueue()
{
    timerfdChannel_.disableAll();
    timerfdChannel_.remove();
    close(timerfd_);
    for(auto& timer:timers_)
    {
        delete timer.second;
    }
}

TimerId TimerQueue::addTimer(TimerCallback cb, Timestamp when, double interval)
{
    Timer* timer = new Timer(std::move(cb), when, interval);
    loop_->runInLoop(std::bind(&TimerQueue::addTimerInLoop,this,timer));
    return  TimerId(timer,timer->sequence());
}

void TimerQueue::addTimerInLoop(Timer *timer)
{
    bool earliestChanged = insert(timer);
    if(earliestChanged)
    {
        resetTimerfd(timerfd_,timer->expiration());
    }
}

void TimerQueue::cancel(TimerId timerId)
{
    loop_->runInLoop(std::bind(&TimerQueue::cancelInLoop,this,timerId));
}

void TimerQueue::cancelInLoop(TimerId timerId)
{
    assert(timers_.size() == activeTimers_.size());
    ActiveTimer timer(timerId.timer,timerId.sequence_);
    auto it = activeTimers_.find(timer);
    if(it != activeTimers_.end())
    {
        timers_.erase(Entry(it->first->expiration(),it->first));
        delete it->first;
        activeTimers_.erase(it);
    }
    else if(callingExpiredTimers_)
    {
        cancelingTimers_.insert(timer);
    }
}

void TimerQueue::handleRead()
{
    Timestamp now(Timestamp::now());
    readTimerfd(timerfd_,now);
    std::vector<Entry> expired = getExpired(now);
    callingExpiredTimers_ = true;
    cancelingTimers_.clear();

    for(auto& it : expired)
    {
        it.second->run();
    }
    callingExpiredTimers_ = false;
    reset(expired,now);
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now)
{
    assert(timers_.size() == activeTimers_.size());
    std::vector<Entry> expired;
    Entry sentry = std::make_pair(now, reinterpret_cast<Timer*>(UINTPTR_MAX));
    auto it_expired = timers_.lower_bound(sentry);
    assert(it_expired == timers_.end() || now < it_expired->first);
    std::copy(timers_.begin(), it_expired, back_inserter(expired));
    timers_.erase(timers_.begin(), it_expired);

    for(auto& it : expired)
    {
        ActiveTimer timer(it.second,it.second->sequence());
        size_t n = activeTimers_.erase(timer);
        assert(n == 1); (void)n;
    }
    assert(timers_.size() == activeTimers_.size());
    return expired;
}

void TimerQueue::reset(const std::vector<Entry> &expired, Timestamp now)
{
    Timestamp nextExpire;

    for(auto& it :expired)
    {
        ActiveTimer timer(it.second,it.second->sequence());
        if(it.second->repeat()
                && cancelingTimers_.find(timer) == cancelingTimers_.end())
        {
            it.second->restart(now);
            insert(it.second);
        }
        else
        {
            delete it.second;
        }
    }
    if(!timers_.empty())
    {
        nextExpire = timers_.begin()->second->expiration();
    }

    if(nextExpire.valid())
    {
        resetTimerfd(timerfd_,nextExpire);
    }
}

bool TimerQueue::insert(Timer *timer)
{
    assert(timers_.size() == activeTimers_.size());
    bool earliestChanged = false;
    Timestamp when = timer->expiration();
    auto it = timers_.begin();
    if(it == timers_.end() || when < it->first)
    {
        earliestChanged = true;
    }
    {
        std::pair<TimerList::iterator, bool> result
                = timers_.insert(Entry(when, timer));
        assert(result.second); (void)result;
    }
    {
        std::pair<ActiveTimerSet::iterator, bool> result
                = activeTimers_.insert(ActiveTimer(timer, timer->sequence()));
        assert(result.second); (void)result;
    }

    assert(timers_.size() == activeTimers_.size());
    return earliestChanged;
}
