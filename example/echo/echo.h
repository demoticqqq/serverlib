//
// Created by huangw on 22-10-22.
//

#ifndef SERVERLIB_ECHO_H
#define SERVERLIB_ECHO_H

#include "serverlib/net/TcpServer.h"
#include "serverlib/base/Timestamp.h"

class EchoServer
{
public:
    EchoServer(EventLoop* loop,
               const InetAddress& listenAddr);
    void start();

private:
    void onConnection(const TcpConnectionPtr& conn);

    void onMessage(const TcpConnectionPtr& conn,
                   Buffer* buf,
                   Timestamp time);

    TcpServer server_;
};


#endif //SERVERLIB_ECHO_H
