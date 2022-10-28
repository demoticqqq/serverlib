//
// Created by huangw on 22-10-15.
//

#ifndef SERVERLIB_CALLBACK_H
#define SERVERLIB_CALLBACK_H

#include <memory>
#include <functional>

class Buffer;
class TcpConnection;
class Timestamp;

using TimerCallback = std::function<void()>;
using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
using ConnectionCallback = std::function<void(const TcpConnectionPtr &)>;
using MessageCallback = std::function<void(const TcpConnectionPtr &, Buffer *,Timestamp)>;
using CloseCallback = std::function<void(const TcpConnectionPtr &)>;
using WriteCompleteCallback = std::function<void(const TcpConnectionPtr &)>;
using HighWaterMarkCallback = std::function<void(const TcpConnectionPtr &,size_t)>;

#endif //SERVERLIB_CALLBACK_H
