//
// Created by huangw on 22-10-26.
//

#ifndef SERVERLIB_TIMERID_H
#define SERVERLIB_TIMERID_H



class Timer;

class TimerId
{
public:
    TimerId()
        : timer(nullptr)
        , sequence_(0)
    {
    }

    TimerId(Timer* timer, int64_t seq)
        : timer(timer)
        , sequence_(seq)
    {
    }
    friend class TimerQueue;
private:
    Timer* timer;
    int64_t sequence_;
};

#endif //SERVERLIB_TIMERID_H