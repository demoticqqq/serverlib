//
// Created by huangw on 22-9-30.
//

#include "CountDownLatch.h"

CountDownLatch::CountDownLatch(int count)
    :mutex_()
    ,condition_()
    ,count_(count)
{
}

void CountDownLatch::wait()
{
    std::unique_lock<std::mutex>lk(mutex_);
    condition_.wait(lk,[&](){
        return count_==0;
    });
}

void CountDownLatch::countDown()
{
    std::unique_lock<std::mutex>lk(mutex_);
    --count_;
    if(count_ == 0)
    {
        condition_.notify_all();
    }
}

int CountDownLatch::getCount() const
{
    std::unique_lock<std::mutex>lk(mutex_);
    return count_;
}