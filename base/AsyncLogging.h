//
// Created by huangw on 22-10-31.
//

#ifndef SERVERLIB_ASYNCLOGGING_H
#define SERVERLIB_ASYNCLOGGING_H
#include "CountDownLatch.h"
#include "Thread.h"
#include "Log_buffer.h"

#include <atomic>
#include "vector"

class AsyncLogging : noncopyable
{
public:
    AsyncLogging(const std::string& basename, off_t rollsize, int flushInterval = 3);
    ~AsyncLogging();

    void append(const char* msg, int len);

    void start();

    void stop();

private:
    void threadFunc();

    using Buffer = FixedBuffer<kLargeBuffer>;
    using BuffetVector = std::vector<std::unique_ptr<Buffer>>;
    using BufferPtr = BuffetVector::value_type;

    const int flushInterval_;
    std::atomic_bool running_;
    const std::string basename_;
    const off_t rollSize_;
    Thread thread_;
    CountDownLatch latch_;
    std::mutex mutex_;
    std::condition_variable cond_;
    BufferPtr currentBuffer_;
    BufferPtr nextBuffer_;
    BuffetVector buffers_;
};


#endif //SERVERLIB_ASYNCLOGGING_H
