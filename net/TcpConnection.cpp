//
// Created by huangw on 22-10-15.
//
//#include <netinet/tcp.h>
#include <functional>

#include "TcpConnection.h"
#include "../base/Logger.h"
#include "Socket.h"
#include "Channel.h"
#include "EventLoop.h"

TcpConnection::TcpConnection(EventLoop *loop,
                             const std::string &nameArg,
                             int sockfd,
                             const InetAddress &localAddr,
                             const InetAddress &peerAddr)
    : loop_(loop)
    , name_(nameArg)
    , socket_(new Socket(sockfd))
    , channel_(new Channel(loop,sockfd))
    , state_(kConnecting)
    , localAddr_(localAddr)
    , peerAddr_(peerAddr)
    , highWaterMark_(64*1024*1024)
{
    channel_->setReadCallback(std::bind(&TcpConnection::handleRead,this,std::placeholders::_1));
    channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
    channel_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
    channel_->setErrorCallback(std::bind(&TcpConnection::handleError, this));

    LOG_INFO("TcpConnection::ctor[%s] at fd=%d\n", name_.c_str(), sockfd);
    socket_->setKeepAlive(true);
}

TcpConnection::~TcpConnection()
{
    LOG_INFO("TcpConnection::dtor[%s] at fd=%d state=%d\n", name_.c_str(), channel_->fd(), (int)state_);
}

void TcpConnection::send(const std::string &buf)
{
    if(state_ == kConnected)
    {
        if(loop_->isInLoopThread())  // 这种对于单个reactor的情况 用户调用conn->send时 loop_即为当前线程
        {
            sendInLoop(buf);
        }
        else    //多reactor情况
        {
            void(TcpConnection:: *fp)(const std::string& buf) = &TcpConnection::sendInLoop;
            loop_->runInLoop(std::bind(fp, this, buf));
        }
    }
}

void TcpConnection::sendInLoop(const std::string &buf)
{
    sendInLoop(buf.data(),buf.size());
}

void TcpConnection::sendInLoop(const void* data,size_t len)
{
    ssize_t nwrote = 0;
    size_t remaining = len;
    bool  faultError = false;
    if(state_ = kDisconnected)  // 之前调用过该connection的shutdown 不能再进行发送了
    {
        LOG_ERROR("disconnected, give up writing");
    }
    // 表示channel_第一次开始写数据或者缓冲区没有待发送数据
    if(!channel_->isWriting() && outputBuffer_.readableBytes() == 0)
    {
        nwrote = write(channel_->fd(),data,len);
        if(nwrote >= 0)
        {
            remaining = len - nwrote;
            if(remaining == 0 && writeCompleteCallback_) // 既然在这里数据全部发送完成，就不用再给channel设置epollout事件了
            {
                loop_->queueInLoop(std::bind(writeCompleteCallback_,shared_from_this()));
            }
        }
        else
        {
            nwrote = 0;
            if(errno == EWOULDBLOCK)  // EWOULDBLOCK表示非阻塞情况下没有数据后的正常返回 等同于EAGAIN
            {
                LOG_ERROR("TcpConnection::sendInLoop");
                if(errno == EPIPE || errno == ECONNRESET) //SIGIPE RESET
                {
                    faultError = true;
                }
            }
        }
    }

    /*说明当前write并没有发送完毕，需要把剩余数据保存到缓冲区，并给channel注册EPOLLOUT事件，
     * 等待Poller通知对应的scokfd->channel_ tcp发送缓存区有空间
     * 并调用channel的writeCallback_,也就是TcpConnection设置的handleWrite回调
     * 将发送缓冲区outputbuffer_的内容全部发送完毕*/
    if(!faultError && remaining)
    {
        size_t oldLen = outputBuffer_.readableBytes();
        //待发送的数据超过水位线，触发高水位回调。
        if(oldLen + remaining >= highWaterMark_ && oldLen < highWaterMark_ && highWaterMarkCallback_)
        {
            loop_->queueInLoop(std::bind(highWaterMarkCallback_,shared_from_this(),oldLen + remaining));
        }
        outputBuffer_.append(static_cast<const char*>(data) + nwrote, remaining);
        if(!channel_->isWriting())
        {
            channel_->enableReading();  //必须注册写事件，这样，Poller才能通知对应的channel。
        }
    }

}

void TcpConnection::shutdown()
{
    if(state_ == kConnected)
    {
        setState(kDisconnected);
        loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
    }
}

void TcpConnection::shutdownInLoop()
{
    if(!channel_->isWriting())
    {
        socket_->shutdownWrite();
    }
}

void TcpConnection::setTcpNoDelay(bool on)
{
    socket_->setTcpNoDelay(on);
}


void TcpConnection::connectEstablished()
{
    setState(kConnected);
    channel_->tie(shared_from_this());
    channel_->enableReading();
    connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestroyed()
{
    if(state_ == kConnected)
    {
        setState(kDisconnected);
        channel_->disableAll();
        connectionCallback_(shared_from_this());
    }
    channel_->remove();

}

void TcpConnection::handleRead(Timestamp receiveTime)
{
    int savedErrno = 0;
    ssize_t n = inputBuffer_.readFd(channel_->fd(),&savedErrno);
    if(n > 0)
    {
        messageCallback_(shared_from_this(),&inputBuffer_,receiveTime);
    }
    else if(n == 0)
    {
        handleClose();
    }
    else
    {
        errno = savedErrno;
        LOG_ERROR("TcpConnection::handleRead");
        handleError();
    }
}

void TcpConnection::handleWrite()
{
    if(channel_->isWriting())
    {
        ssize_t n = write(channel_->fd(),outputBuffer_.peek(),outputBuffer_.readableBytes());
        if(n > 0)
        {
            outputBuffer_.retrieve(n);
            if(outputBuffer_.readableBytes() == 0)
            {
                channel_->disableWriting();
                if(writeCompleteCallback_)
                {
                    loop_->queueInLoop(std::bind(writeCompleteCallback_,shared_from_this()));
                }
                if(state_ == kDisconnecting)
                {
                    shutdownInLoop();
                }
            }
        }
        else
        {
            LOG_ERROR("TcpConnection::handleWrite");
        }
    }
    else
    {
        LOG_ERROR("TcpConnection fd=%d is down, no more writing", channel_->fd());
    }
}

void TcpConnection::handleClose()
{
    LOG_INFO("TcpConnection::handleClose fd=%d state=%d\n",channel_->fd(),static_cast<int>(state_));
    setState(kDisconnected);
    channel_->disableAll();

    TcpConnectionPtr connPtr(shared_from_this());
    connectionCallback_(connPtr);  //执行关闭连接的回调
    closeCallback_(connPtr);   //执行关闭连接的回调，调用的是TcpServer::removeConnection的方法
}

void TcpConnection::handleError()
{
    int optval;
    socklen_t optlen = static_cast<socklen_t>(sizeof optval);
    int err = 0;
    if(getsockopt(channel_->fd(),SOL_SOCKET,SO_ERROR,&optval,&optlen))
    {
        err = errno;
    }
    else
    {
        err = optval;
    }
    LOG_ERROR("TcpConnection::handleError name:%s - SO_ERROR:%d\n", name_.c_str(), err);
}