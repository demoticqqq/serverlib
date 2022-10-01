//
// Created by huangw on 22-10-1.
//

#include "ThreadPool.h"

ThreadPool::ThreadPool(const std::string &nameArg)
    :mutex_()
    ,notEmpty_()
    ,notFull_()
    ,name_(nameArg)
    ,maxQueueSize_(0)
    ,running_(false)
{
}

ThreadPool::~ThreadPool()
{
    if(running_)
    {
        stop();
    }
}

void ThreadPool::start(int numThreads)
{
    running_=true;
    threads_.reserve(numThreads);
    for(int i = 0; i < numThreads; i++)
    {
        char id[32];
        snprintf(id,32,"%d",i+1);
        threads_.emplace_back(new Thread(std::bind(&ThreadPool::runInThread, this),name_+id));
        threads_[i]->start();
    }
    if(numThreads == 0 && threadInitCallback_)
    {
        threadInitCallback_();
    }
}

void ThreadPool::stop()
{
    {
        std::lock_guard<std::mutex>lk(mutex_);
        running_ = false;
        notEmpty_.notify_all();
        notFull_.notify_all();
    }
    for(auto& th:threads_)
    {
        th->join();
    }
}

size_t ThreadPool::queueSize() const
{
    std::lock_guard<std::mutex>lk(mutex_);
    return queue_.size();
}

void ThreadPool::run(ThreadPool::Task task)
{
    if(threads_.empty())
    {
        task();
    }
    else
    {
        std::unique_lock<std::mutex>lk(mutex_);
        notFull_.wait(lk,[&](){
            return !(isFull() && running_);
        });
        if(!running_) return;

        queue_.push_back(std::move(task));
        notEmpty_.notify_one();
    }
}

ThreadPool::Task ThreadPool::take()
{
    std::unique_lock<std::mutex>lk(mutex_);
    notEmpty_.wait(lk,[&](){
        return (!queue_.empty()) || !running_;
    });
    Task task;
    if(!queue_.empty())
    {
        task = queue_.front();
        queue_.pop_front();
        if(maxQueueSize_>0)
        {
            notFull_.notify_one();
        }
    }
    return task;
}

bool ThreadPool::isFull() const
{
    return maxQueueSize_ > 0 && queue_.size() >= maxQueueSize_;
}

void ThreadPool::runInThread()
{
    try
    {
        if(threadInitCallback_)
        {
            threadInitCallback_();
        }
        while(running_)
        {
            Task task(take());
            if(task)
            {
                task();
            }
        }
    }
    catch (...)
    {
        fprintf(stderr, "unknown exception caught in ThreadPool %s\n", name_.c_str());
        throw;
    }
}