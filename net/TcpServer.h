//
// Created by huangw on 22-10-20.
//

#ifndef SERVERLIB_TCPSERVER_H
#define SERVERLIB_TCPSERVER_H

#include <map>

#include "TcpConnection.h"

class Acceptor;
class EventLoopThreadPool;

class TcpServer : noncopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop *)>;

    enum Option
    {
        kNoReusePort,
        kReusePort
    };
    TcpServer(EventLoop* loop,
              const InetAddress& listenAddr,
              const std::string& nameArg,
              Option option = kNoReusePort);
    ~TcpServer();

    const std::string& ipPort() {return ipPort_;}
    const std::string& name() {return name_;}
    EventLoop* getLoop() const {return loop_;}
    std::shared_ptr<EventLoopThreadPool> threadPool() {return threadPool_;}

    void setThreadNum(int numThread);
    void setThreadInitCallback(const ThreadInitCallback& cb) {threadInitCallback_ = cb;}

    void start();

    void setConnectionCallback(const ConnectionCallback& cb) {connectionCallback_ = cb;}
    void setMessageCallback(const MessageCallback& cb) {messageCallback_ = cb;}
    void setWriteCompleteCallback(const WriteCompleteCallback& cb) {writeCompleteCallback_ = cb;}


private:
    void newConnection(int sockfd, const InetAddress& peerAddr);

    void removeConnection(const TcpConnectionPtr& conn);

    void removeConnectionInLoop(const TcpConnectionPtr& conn);

    using ConnectionMap = std::map<std::string,TcpConnectionPtr>;

    EventLoop* loop_;  //baseloop

    const std::string ipPort_;
    const std::string name_;
    std::unique_ptr<Acceptor> acceptor_;
    std::shared_ptr<EventLoopThreadPool> threadPool_;

    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    ThreadInitCallback threadInitCallback_;
    std::atomic_int started_;
    int nextConnId_;
    ConnectionMap connections_;
};


#endif //SERVERLIB_TCPSERVER_H
