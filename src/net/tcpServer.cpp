#include "tcpServer.h"
#include "Acceptor.h"
#include "EventLoopThreadPool.h"
#include "Socket.h"
#include "tcpConnection.h"
#include <atomic>
namespace adl {

TcpServer::TcpServer(const std::shared_ptr<EventLoop> &loop,
                     const InetAddress &listenAddr)
    : mainLoop_(loop), ip_(listenAddr.toIp()), port_(listenAddr.toPortString()),
      acceptor_(std::make_unique<Acceptor>(mainLoop_, listenAddr)),
      threadPool_(std::make_shared<EventLoopThreadPool>(mainLoop_)),
      connectionCallback_(defaultConnectionCallback),
      messageCallback_(defaultMessageCallback) {
  /* acceptor new Connect call TcpServer::newConnection  */
  acceptor_->setNewConnectionCallback(
      std::bind(&TcpServer::newConnection, this, _1, _2));
}

TcpServer::~TcpServer() {
  mainLoop_->assertInLoopThread();
  /* LOG */
  // for (auto &item : connections_) {
  //   TcpConnectionPtr conn(item.second);
  //   item.second.reset();
  //   conn->getLoop()->runInLoop(
  //       std::bind(&TcpConnection::connectDestroyed, conn));
  // }
}

/* 开始跑
  EventLoopThreadPool开始创建多个EventLoop线程进行跑，
  acceptor开始listen
 */
void TcpServer::start() {
  if (!started_.load(std::memory_order_relaxed)) {
    bool expected = false;
    started_.compare_exchange_strong(expected, true);
  }
  threadPool_->start(threadInitCallback_);
  assert(!acceptor_->listening());
  mainLoop_->runInLoop(std::bind(&Acceptor::listen, acceptor_.get()));
}

void TcpServer::setThreadNum(int numThreads) {
  assert(0 <= numThreads);
  threadPool_->setThreadNum(numThreads);
}

/* 作为acceptor的回调函数:
  创建新的TcpConnection对象,绑定一个subLoop,
  设置各种回调函数，
  并让subLoop执行连接建立回调函数 */
void TcpServer::newConnection(int sockfd, const InetAddress &peerAddr) {
  mainLoop_->assertInLoopThread();
  std::shared_ptr<EventLoop> subLoop = threadPool_->getNextLoop();
  /* LOG */
  InetAddress localAddr(sock::getLocalAddr(sockfd));
  TcpConnectionPtr conn(
      new TcpConnection(subLoop, sockfd, localAddr, peerAddr));
  conn->setConnectionCallback(connectionCallback_);
  conn->setMessageCallback(messageCallback_);
  conn->setWriteCompleteCallback(writeCompleteCallback_);
  conn->setCloseCallback(
      std::bind(&TcpServer::removeConnection, this, _1)); // FIXME: unsafe
  subLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}

/* 作为 tcpConnection 的连接断开回调函数，删除一个连接
  有点奇葩：tcpConnection在某个线程执行了closeCallBack
  会在 mainLoop中 tcpServer::removeConnection，
  然后会在 subLoop中执行TcpConnection::connectDestroyed,
  然后 让channel disableAll 和 remove 从EventLoop中删除。
*/
void TcpServer::removeConnection(const TcpConnectionPtr &conn) {
  // FIXME: unsafe
  mainLoop_->runInLoop(
      std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn) {
  mainLoop_->assertInLoopThread();
  // LOG_INFO << "TcpServer::removeConnectionInLoop [" << name_
  //          << "] - connection " << conn->name();
  // size_t n = connections_.erase(conn->name());

  std::shared_ptr<EventLoop> subLoop = conn->getLoop();
  subLoop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}

} // namespace adl
