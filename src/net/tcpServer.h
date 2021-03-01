#ifndef TCPSERVER_H
#define TCPSERVER_H
#include "Acceptor.h"
#include "EventLoopThreadPool.h"
#include "InetAddress.h"
#include "callBack.h"
#include <atomic>
#include <boost/noncopyable.hpp>
namespace adl {

class TcpServer : boost::noncopyable {
public:
  using ThreadInitCallback = EventLoopThread::ThreadInitCallback;
  TcpServer(const std::shared_ptr<EventLoop> &loop,
            const InetAddress &listenAddr, int numThreads = 4);
  ~TcpServer();

  void start();

  const std::string &getip() const { return ip_; }
  const std::string &getport() const { return port_; }

  /* 返回main Loop */
  std::shared_ptr<EventLoop> getLoop() const { return mainLoop_; }

  /* 设置线程数量 */
  void setThreadNum(int numThreads);

  void setConnectionCallback(ConnectionCallback &&cb) {
    connectionCallback_ = std::move(cb);
  }

  void setMessageCallback(MessageCallback &&cb) {
    messageCallback_ = std::move(cb);
  }

  void setWriteCompleteCallback(WriteCompleteCallback &&cb) {
    writeCompleteCallback_ = std::move(cb);
  }
  void setThreadInitCallback(const ThreadInitCallback &&cb) {
    threadInitCallback_ = std::move(cb);
  }
  void removeConnection(const TcpConnectionPtr &conn);
  void removeConnectionInLoop(const TcpConnectionPtr &conn);

  void newConnection(int sockfd, const InetAddress &peerAddr);

private:
  std::shared_ptr<EventLoop> mainLoop_; // main loop
  const std::string ip_;                // local ip
  const std::string port_;              // local port
  std::unique_ptr<Acceptor> acceptor_; /*  接受者 handleRead 调用 accept */

  std::unique_ptr<EventLoopThreadPool>
      threadPool_; /* EventLoopThreadPool 用来分发subLoop连接 */

  ConnectionCallback connectionCallback_;
  MessageCallback messageCallback_;
  WriteCompleteCallback writeCompleteCallback_;
  ThreadInitCallback threadInitCallback_;

  // ConnectionMap connections_;

  std::atomic<bool> started_; /* 是否服务器已经开始跑 */
};
} // namespace adl

#endif
