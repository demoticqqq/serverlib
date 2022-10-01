//
// Created by huangw on 22-10-1.
//

#ifndef SERVERLIB_THREADPOOL_H
#define SERVERLIB_THREADPOOL_H
#include "noncopyable.h"
#include "Thread.h"
#include <mutex>
#include <condition_variable>
#include <vector>
#include <deque>

class ThreadPool:noncopyable
{
public:
    using Task = std::function<void()>;

    explicit ThreadPool(const string& nameArg = string("ThreadPool"));
    ~ThreadPool();

    void setMaxQueueSize(int maxSize) { maxQueueSize_=maxSize;}
    void setThreadInitCallback(const Task& cb) {threadInitCallback_=cb;}

    void start(int numThreads);
    void stop();

    const std::string& name() const {return name_;}

    size_t queueSize() const;

    void run(Task task);

private:
    bool isFull() const;
    void runInThread();
    Task take();

    mutable std::mutex mutex_;
    std::condition_variable notEmpty_;
    std::condition_variable notFull_;
    std::string name_;
    Task threadInitCallback_;
    std::vector<std::unique_ptr<Thread>> threads_;
    std::deque<Task>queue_;
    size_t maxQueueSize_;
    bool running_;


};


#endif //SERVERLIB_THREADPOOL_H
