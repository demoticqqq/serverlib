//
// Created by huangw on 22-10-26.
//

#ifndef SERVERLIB_TIMER_H
#define SERVERLIB_TIMER_H

#include "../base/Timestamp.h"
#include "../base/noncopyable.h"
#include "Callback.h"
#include <atomic>

class Timer : noncopyable
{
public:
    Timer(const TimerCallback cb, Timestamp when, double interval)
        : callback_(std::move(cb))
        , expiration_(when)
        , interval_(interval)
        , repeat_(interval > 0.0)
        , sequence_(++s_numCreated_)

    {
    }

    void run() const {callback_();}

    Timestamp expiration() const {return expiration_;}
    bool repeat() const {return repeat_;}
    int64_t sequence() const {return sequence_;}

    void restart(Timestamp now);

    static int64_t numCreated() {return s_numCreated_;}

private:
    const TimerCallback callback_;
    Timestamp expiration_;
    const double interval_;
    const bool repeat_;

    const int64_t sequence_;  //定时器序号
    static std::atomic_int64_t s_numCreated_; //定时器计数，当前已创建的定时器数量

};


#endif //SERVERLIB_TIMER_H
