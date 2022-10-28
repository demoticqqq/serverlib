cmake_minimum_required(VERSION 3.16)
project(serverlib)

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

set(CMAKE_CXX_STANDARD 11)

add_library(serverlib SHARED base/Thread.h base/Thread.cpp base/noncopyable.h base/CurrentThread.h base/CountDownLatch.cpp base/CountDownLatch.h net/EventLoop.cpp net/EventLoop.h base/Timestamp.cpp base/Timestamp.h base/ThreadPool.cpp base/ThreadPool.h net/Channel.cpp net/Channel.h net/Poller.cpp net/Poller.h net/EpollPoller.cpp net/EpollPoller.h base/Logger.cpp base/Logger.h net/Socket.cpp net/Socket.h net/InetAddress.cpp net/InetAddress.h net/EventLoopThread.cpp net/EventLoopThread.h net/EventLoopThreadPool.cpp net/EventLoopThreadPool.h net/Acceptor.cpp net/Acceptor.h net/Buffer.cpp net/Buffer.h net/TcpConnection.cpp net/TcpConnection.h net/Callback.h net/TcpServer.cpp net/TcpServer.h base/CurrentThread.cpp net/DefaultPoller.cpp net/TimerQueue.cpp net/TimerQueue.h net/Timer.cpp net/Timer.h net/TimerId.h)
