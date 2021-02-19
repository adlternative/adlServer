#ifndef TCPSERVER_H
#define TCPSERVER_H
#include "EventLoopThreadPool.h"
#include "InetAddress.h"
#include <atomic>

#include <boost/noncopyable.hpp>
namespace adl {

class TcpServer : boost::noncopyable {
public:
  typedef std::function<void(EventLoop *)> ThreadInitCallback;
  TcpServer(EventLoop *loop, const InetAddress &listenAddr);
  ~TcpServer();

private:
  EventLoop *loop_; // main loop
  std::string ip;
  std::string port_;
  /* ACCEPTOR */
  std::shared_ptr<EventLoopThreadPool> threadPool_;
  // ConnectionCallback connectionCallback_;
  // MessageCallback messageCallback_;
  // WriteCompleteCallback writeCompleteCallback_;

  // ConnectionMap connections_;

  std::atomic<bool> started_;
};
} // namespace adl

#endif
