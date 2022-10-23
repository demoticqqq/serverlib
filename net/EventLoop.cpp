//
// Created by huangw on 22-10-1.
//
#include <sys/eventfd.h>
#include "EventLoop.h"
#include "Channel.h"
#include "Poller.h"
#include "../base/Logger.h"

__thread EventLoop* t_loopInThisThread = nullptr;

const int kPollerTimeoutMs = 10000; //10s,poller超时时间

int createEventfd()
{
    int evfd = eventfd(0,EFD_NONBLOCK | EFD_CLOEXEC);
    if (evfd < 0)
    {
        LOG_FATAL("eventfd create error:%d\n",errno);
    }
    return evfd;
}

EventLoop::EventLoop()
    :looping_(false)
    ,quit_(false)
    ,threadId_(CurrentThread::tid())
    ,callingPendingFunctors_(false)
    ,poller_(Poller::newDefaultPoller(this))
    //,timerQueue_()
    ,wakeupFd_(createEventfd())
    ,wakeupChannel_(new Channel(this,wakeupFd_))
{
    LOG_DEBUG("EventLoop created %p in thread %d\n",this,threadId_);
    if(t_loopInThisThread)
    {
        LOG_FATAL("Another EventLoop %p exists in this Thread %d\n",t_loopInThisThread,threadId_);
    }
    else
    {
        t_loopInThisThread = this;
    }
    wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead,this));
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop()
{
    LOG_DEBUG("EventLoop %p of thread %d destructs in thread %d\n",this,threadId_,CurrentThread::tid());
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    ::close(wakeupFd_);
    t_loopInThisThread = nullptr;
}

void EventLoop::loop()
{
    looping_ = true;
    quit_ = false;
    LOG_INFO("EventLoop %p start looping\n",this);

    while (!quit_)
    {
        activeChannels_.clear();
        pollReturnTime_ = poller_->poll(kPollerTimeoutMs,&activeChannels_);
        for(auto& channel:activeChannels_)
        {
            channel->handleEvent(pollReturnTime_);
        }
        /**
        * 执行当前EventLoop事件循环需要处理的回调操作 对于线程数 >=2 的情况 IO线程 mainloop(mainReactor) 主要工作：
        * accept接收连接 => 将accept返回的connfd打包为Channel => TcpServer::newConnection通过轮询将TcpConnection对象分配给subloop处理
        *
        * mainloop调用queueInLoop将回调加入subloop（该回调需要subloop执行 但subloop还在poller_->poll处阻塞） queueInLoop通过wakeup将subloop唤醒
        **/
        doPendingFuncTors();
    }
    LOG_INFO("EventLoop %p stop looping\n",this);
    looping_ = false;
}
/**
 * 退出事件循环
 * 1. 如果loop在自己的线程中调用quit成功了 说明当前线程已经执行完毕了loop()函数的poller_->poll并退出
 * 2. 如果不是当前EventLoop所属线程中调用quit退出EventLoop 需要唤醒EventLoop所属线程的epoll_wait
 *
 * 比如在一个subloop(worker)中调用mainloop(IO)的quit时 需要唤醒mainloop(IO)的poller_->poll 让其执行完loop()函数
 *
 * ！！！ 注意： 正常情况下 mainloop负责请求连接 将回调写入subloop中 通过生产者消费者模型即可实现线程安全的队列
 * ！！！       但是muduo通过wakeup()机制 使用eventfd创建的wakeupFd_ notify 使得mainloop和subloop之间能够进行通信
 **/
void EventLoop::quit()
{
    quit_ = true;
    if(!isInLoopThread())
    {
        wakeup();
    }
}

void EventLoop::runInLoop(EventLoop::Functor cb)
{
    if(isInLoopThread())
    {
        cb();
    }
    else
    {
        queueInLoop(std::move(cb));
    }
}

void EventLoop::queueInLoop(EventLoop::Functor cb)
{
    {
        std::unique_lock<std::mutex>lk(mutex_);
        pendingFunctors_.emplace_back(std::move(cb));
    }
    /**
     * || callingPendingFunctors的意思是 当前loop正在执行回调中 但是loop的pendingFunctors_中又加入了新的回调 需要通过wakeup写事件
     * 唤醒相应的需要执行上面回调操作的loop的线程 让loop()下一次poller_->poll()不再阻塞（阻塞的话会延迟前一次新加入的回调的执行），然后
     * 继续执行pendingFunctors_中的回调函数
     **/
    if(!isInLoopThread() || callingPendingFunctors_)
    {
        wakeup();
    }
}

size_t EventLoop::queueSize() const
{
    std::unique_lock<std::mutex>lk(mutex_);
    return pendingFunctors_.size();
}

void EventLoop::updateChannel(Channel *channel)
{
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel *channel)
{
    poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel *channel)
{
    return poller_->hasChannel(channel);
}

void EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = write(wakeupFd_,&one,sizeof one);
    if(n != sizeof one)
    {
        LOG_ERROR("EventLoop::wakeup() writes %d bytes instead of 8\n",n);
    }
}

void EventLoop::handleRead()
{
    uint64_t one = 1;
    ssize_t n = read(wakeupFd_,&one,sizeof one);
    if(n != sizeof one)
    {
        LOG_ERROR("EventLoop::hanleRead() reads %d bytes instead of 8\n",n);
    }
}

void EventLoop::doPendingFuncTors()
{
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;
    {
        std::unique_lock<std::mutex>lk(mutex_);
        functors.swap(pendingFunctors_);
    }
    for(const Functor& functor:functors)
    {
        functor();
    }
    callingPendingFunctors_ = false;
}

