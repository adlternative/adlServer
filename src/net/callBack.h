#ifndef CALLBACK_H
#define CALLBACK_H
#include <functional>
namespace adl {

typedef std::function<void()> TimerCallback;
// typedef std::function<void(const TcpConnectionPtr &)> ConnectionCallback;
// typedef std::function<void(const TcpConnectionPtr &)> CloseCallback;
// typedef std::function<void(const TcpConnectionPtr &)> WriteCompleteCallback;
} // namespace adl
#endif
