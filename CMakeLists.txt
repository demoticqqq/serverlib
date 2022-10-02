cmake_minimum_required(VERSION 3.23)
project(serverlib)

set(CMAKE_CXX_STANDARD 11)

add_library(serverlib base/Thread.h base/Thread.cpp base/noncopyable.h base/CurrentThread.h base/CountDownLatch.cpp base/CountDownLatch.h net/EventLoop.cpp net/EventLoop.h base/Timestamp.cpp base/Timestamp.h base/ThreadPool.cpp base/ThreadPool.h net/Channel.cpp net/Channel.h net/Poller.cpp net/Poller.h net/EpollPoller.cpp net/EpollPoller.h base/Logger.cpp base/Logger.h)
