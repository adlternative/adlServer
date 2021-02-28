#ifndef CALLBACK_H
#define CALLBACK_H
#include "../base/timeStamp.h"
#include <functional>
#include <memory>
namespace adl {
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
class netBuffer;
class TcpConnection;
typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
typedef std::function<void()> TimerCallback;
typedef std::function<void(const TcpConnectionPtr &)> ConnectionCallback;
typedef std::function<void(const TcpConnectionPtr &)> CloseCallback;
typedef std::function<void(const TcpConnectionPtr &)> WriteCompleteCallback;
typedef std::function<void(const TcpConnectionPtr &,
                           netBuffer * /*, timeStamp */)>
    MessageCallback;
} // namespace adl
#endif
