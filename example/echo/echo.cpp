//
// Created by huangw on 22-10-22.
//

#include "echo.h"
#include <string>
#include "serverlib/base/Logger.h"

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

EchoServer::EchoServer(EventLoop *loop, const InetAddress &listenAddr)
    : server_(loop,listenAddr,"EchoServer")
{
    server_.setConnectionCallback(std::bind(&EchoServer::onConnection,this,_1));

    server_.setMessageCallback(std::bind(&EchoServer::onMessage,this,_1,_2,_3));

}

void EchoServer::start()
{
    server_.start();
}

void EchoServer::onConnection(const TcpConnectionPtr &conn)
{
    LOG_INFO("EchoServer - %s -> %s is %s\n",
             conn->peerAddress().toIpPort().c_str(),conn->localAddress().toIpPort().c_str(),(conn->connected()?"UP":"DOWN"));
}

void EchoServer::onMessage(const TcpConnectionPtr &conn, Buffer* buf, Timestamp time)
{
    std::string msg = buf->retrieveAllAsString();
    LOG_INFO("%s echo %d bytes, data received at %s\n",
             conn->name().c_str(),msg.size(),time.toString().c_str());
    conn->send(msg);
}