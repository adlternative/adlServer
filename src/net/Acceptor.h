#ifndef ACCEPTOR_H
#define ACCEPTOR_H
#include "Channel.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "Socket.h"
#include <boost/noncopyable.hpp>
namespace adl {

class Acceptor : boost::noncopyable {
  Acceptor(EventLoop *loop, const InetAddress &listenAddr);
  ~Acceptor();
  void listen();

private:
  void handleRead();
  EventLoop *loop_;
  Socket acceptSocket_;
  Channel acceptChannel_;
  std::atomic<bool> listening_;
};

} // namespace adl
#endif
