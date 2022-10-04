//
// Created by huangw on 22-10-4.
//

#ifndef SERVERLIB_EVENTLOOPTHREADPOOL_H
#define SERVERLIB_EVENTLOOPTHREADPOOL_H

#include "../base/noncopyable.h"

#include <functional>
#include <memory>
#include <vector>

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : noncopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    EventLoopThreadPool(EventLoop* baseloop,const std::string &name);
    ~EventLoopThreadPool();

    void setThreadNum(int numThreads) { numThreads_ = numThreads; }
    void start(const ThreadInitCallback& cb = ThreadInitCallback());

    // 在start（）后调用才有效
    EventLoop* getNextLoop();

    /// with the same hash code, it will always return the same EventLoop
    EventLoop* getLoopForHash(size_t hashCode);

    std::vector<EventLoop*> getAllLoops();

    bool started() const
    { return started_; }

    const std::string& name() const
    { return name_; }


private:
    EventLoop* baseLoop_;
    std::string name_;
    bool started_;
    int numThreads_;
    int next_;
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop*> loops_;

};


#endif //SERVERLIB_EVENTLOOPTHREADPOOL_H
