//
// Created by huangw on 22-9-30.
//
#include "Thread.h"
#include "CountDownLatch.h"
#include "CurrentThread.h"

pid_t gettid()
{
    return static_cast<pid_t>(::syscall(SYS_gettid));
}

void CurrentThread::cachedTid()
{
    if(t_cachedTid==0)
    {
        t_cachedTid=gettid();
    }
}
bool CurrentThread::isMainThread()
{
    return tid()==::gettid();
}

std::atomic_int Thread::numCreated_(0);

Thread::Thread(Thread::ThreadFunc func, const std::string &name)
    :started_(false)
    ,joined_(false)
    ,tid_(0)
    ,func_(std::move(func))
    ,name_(name)
    ,latch_(1)
{
    setDefaultName();
}

Thread::~Thread()
{
    if(started_ && !joined_)
    {
        thread_->detach();
    }
}

void Thread::start()
{
    assert(!started_);
    started_ = true;
    thread_=std::shared_ptr<std::thread>(new std::thread([&](){
        tid_=CurrentThread::tid();
        latch_.countDown();
        func_();
    }));
    latch_.wait();
    assert(tid_>0);
}

void Thread::join()
{
    assert(started_);
    assert(!joined_);
    joined_=true;
    thread_->join();
}

void Thread::setDefaultName()
{
    int num = ++numCreated_;
    if(name_.empty())
    {
        char buf[32];
        snprintf(buf,sizeof(buf),"Thread%d",num);
        name_=buf;
    }
}


