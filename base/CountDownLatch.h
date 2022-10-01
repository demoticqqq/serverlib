//
// Created by huangw on 22-9-30.
//

#ifndef SERVERLIB_COUNTDOWNLATCH_H
#define SERVERLIB_COUNTDOWNLATCH_H
#include "noncopyable.h"
#include <mutex>
#include <condition_variable>
class CountDownLatch : noncopyable
{
public:
    explicit CountDownLatch(int count);

    void wait();

    void countDown();

    int getCount() const;

private:
    mutable std::mutex mutex_;
    std::condition_variable condition_;
    int count_;
};


#endif //SERVERLIB_COUNTDOWNLATCH_H
