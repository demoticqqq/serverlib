//
// Created by huangw on 22-10-31.
//

#include "AsyncLogging.h"
#include "LogFile.h"
#include "Timestamp.h"

AsyncLogging::AsyncLogging(const std::string &basename, off_t rollsize, int flushInterval)
    : flushInterval_(flushInterval)
    , running_(false)
    , basename_(basename)
    , rollSize_(rollsize)
    , thread_(std::bind(&AsyncLogging::threadFunc,this),"logging")
    , latch_(1)
    , mutex_()
    , cond_()
    , currentBuffer_(new Buffer)
    , nextBuffer_(new Buffer)
    , buffers_()
{
    currentBuffer_->bZero();
    nextBuffer_->bZero();
    buffers_.reserve(16);
}

AsyncLogging::~AsyncLogging()
{
    if(running_)
    {
        stop();
    }
}

void AsyncLogging::start()
{
    running_ = true;
    thread_.start();
    latch_.wait();
}

void AsyncLogging::stop()
{
    running_ = false;
    cond_.notify_one();
    thread_.join();
}

void AsyncLogging::append(const char *msg, int len)
{
    std::unique_lock<std::mutex>lk(mutex_);
    if(currentBuffer_->avail() > len)
    {
        currentBuffer_->append(msg, len);
    }
    else
    {
        buffers_.push_back(std::move(currentBuffer_));
        if(nextBuffer_)
        {
            currentBuffer_ = std::move(nextBuffer_);
        }
        else
        {
            currentBuffer_.reset(new Buffer);
        }
        currentBuffer_->append(msg, len);
        cond_.notify_one();
    }
}

void AsyncLogging::threadFunc()
{
    latch_.countDown();
    LogFile output(basename_, rollSize_, false);
    BufferPtr newBuffer1(new Buffer);
    BufferPtr newBuffer2(new Buffer);
    newBuffer1->bZero();
    newBuffer2->bZero();
    BuffetVector buffersToWrite;
    buffersToWrite.reserve(16);
    while (running_)
    {
        {
            std::unique_lock<std::mutex>lk(mutex_);
            if(buffers_.empty())
            {
                cond_.wait_for(lk,std::chrono::seconds(flushInterval_),
                               [&]{return !buffers_.empty();});
            }
            buffers_.push_back(std::move(currentBuffer_));
            currentBuffer_ = std::move(newBuffer1);
            buffersToWrite.swap(buffers_);
            if(nextBuffer_)
            {
                nextBuffer_ = std::move(newBuffer2);
            }
        }

        if(buffersToWrite.size() > 25)
        {
            char buf[256];
            snprintf(buf,sizeof buf,"Dropped log messages at %s, %zd larger buffers\n",
                     Timestamp::now().toString().c_str(),buffersToWrite.size()-2);
            fputs(buf,stderr);
            output.append(buf,static_cast<int>(strlen(buf)));
            buffersToWrite.erase(buffersToWrite.begin()+2,buffersToWrite.end());
        }
        for(const auto& buffer : buffersToWrite)
        {
            output.append(buffer->data(),buffer->length());
        }

        if(buffersToWrite.size() > 2)
        {
            buffersToWrite.resize(2);
        }
        if(!newBuffer1 && !buffersToWrite.empty())
        {
            newBuffer1 = std::move(buffersToWrite.back());
            buffersToWrite.pop_back();
            newBuffer1->reset();
        }
        if(!newBuffer2 && !buffersToWrite.empty())
        {
            newBuffer2 = std::move(buffersToWrite.back());
            buffersToWrite.pop_back();
            newBuffer2->reset();
        }
        buffersToWrite.clear();
        output.flush();
    }
    output.flush();
}