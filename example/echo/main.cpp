//
// Created by huangw on 22-10-22.
//
#include "echo.h"
#include "serverlib/net/TcpServer.h"
#include "serverlib/base/Logger.h"
#include "serverlib/net/EventLoop.h"
#include <unistd.h>

int main()
{
    LOG_INFO("pid = %d\n",getpid());
    EventLoop loop;
    InetAddress listenAddr(8080);
    EchoServer server(&loop,listenAddr);
    server.start();
    loop.loop();

    return 0;
}