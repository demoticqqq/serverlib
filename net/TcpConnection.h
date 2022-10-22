//
// Created by huangw on 22-10-15.
//

#ifndef SERVERLIB_TCPCONNECTION_H
#define SERVERLIB_TCPCONNECTION_H

#include <memory>
#include <atomic>

#include "../base/noncopyable.h"
#include "InetAddress.h"
#include "Callback.h"
#include "Buffer.h"

class Channel;
class EventLoop;
class Socket;

class TcpConnection : noncopyable, public std::enable_shared_from_this<TcpConnection>
{
public:
    TcpConnection(EventLoop *loop,
                  const std::string &nameArg,
                  int sockfd,
                  const InetAddress &localAddr,
                  const InetAddress &peerAddr);
    ~TcpConnection();

    EventLoop* getLoop() const {return loop_;}
    const std::string& name() const {return name_;}
    const InetAddress& localAddress() {return localAddr_;}
    const InetAddress& peerAddress() {return peerAddr_;}
    bool connected() const {return state_ == kConnected;}

    void setConnectionCallback(const ConnectionCallback &cb) {connectionCallback_ = cb;}
    void setMessageCallback(const MessageCallback &cb) {messageCallback_ = cb;}
    void setCloseCallback(const CloseCallback &cb) {closeCallback_ = cb;}
    void setWriteCompleteCallback(const WriteCompleteCallback &cb) {writeCompleteCallback_ = cb;}
    void setHighWaterMarkCallback(const HighWaterMarkCallback &cb,size_t hightWaterMark)
    {highWaterMarkCallback_ = cb; highWaterMark_ = hightWaterMark;}

    //建立连接
    void connectEstablished();
    //销毁连接
    void connectDestroyed();
    //发送数据
    void send(const std::string &buf);
    //关闭连接（半关闭）
    void shutdown();

    void setTcpNoDelay(bool on);

private:
    enum StateE
    {
        kConnecting,  //正在连接
        kConnected,   //已连接
        kDisconnected, //已断开连接
        kDisconnecting //正在断开连接
    };

    void setState(StateE state) {state_ = state;}
    void handleRead(Timestamp receiveTime);
    void handleWrite();
    void handleClose();
    void handleError();

    void sendInLoop(const std::string &buf);
    void sendInLoop(const void* data, size_t len);
    void shutdownInLoop();

    EventLoop *loop_;
    const std::string name_;
    std::atomic_int state_;
    bool reading_;

    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;

    const InetAddress localAddr_;
    const InetAddress peerAddr_;

    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    CloseCallback closeCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    HighWaterMarkCallback highWaterMarkCallback_;
    size_t highWaterMark_;

    Buffer inputBuffer_;
    Buffer outputBuffer_;

};


#endif //SERVERLIB_TCPCONNECTION_H
