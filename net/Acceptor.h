 //
// Created by huangw on 22-10-13.
//

#ifndef SERVERLIB_ACCEPTOR_H
#define SERVERLIB_ACCEPTOR_H
#include <functional>

#include "Socket.h"
#include "Channel.h"

class EventLoop;
class InetAddress;
class Acceptor : noncopyable
{
public:
    using NewConnectCallback = std::function<void(int scokfd,const InetAddress&)>;

    Acceptor(EventLoop* loop,const InetAddress& listenAddr,bool reuseport);
    ~Acceptor();

    void setNewConnectionCallback(const NewConnectCallback& cb) {newConnectCallback_ = cb;}

    void listen();

    bool listening() const {return listening_;}

private:
    void handleRead();

    EventLoop *loop_; //baseLoop 用于接收新连接
    Socket acceptSocket_;   //Socket 内部维护一个sockfd
    Channel acceptChannel_;   //fd和channel成对出现
    NewConnectCallback newConnectCallback_;
    bool listening_;
    int idleFd;  //提前占用一个文件描述符，当文件描述符用尽时，可以利用该文件描述符优雅地关闭连接

};


#endif //SERVERLIB_ACCEPTOR_H
