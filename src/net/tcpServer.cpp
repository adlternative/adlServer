#include "tcpServer.h"
#include "../headFile.h"
#include "../log/Logging.h"
#include "Acceptor.h"
#include "EventLoopThreadPool.h"
#include "Socket.h"
#include "tcpConnection.h"
#include <atomic>
namespace adl {

TcpServer::TcpServer(const std::shared_ptr<EventLoop> &loop,
                     const InetAddress &listenAddr, int numThreads)
    : mainLoop_(loop), ip_(listenAddr.toIp()), port_(listenAddr.toPortString()),
      connectionCallback_(defaultConnectionCallback),
      messageCallback_(defaultMessageCallback) {

  mainLoop_->init();
  acceptor_ = std::make_unique<Acceptor>(mainLoop_, listenAddr);
  threadPool_ = std::make_unique<EventLoopThreadPool>(mainLoop_);
  threadPool_->setThreadNum(numThreads);
  /* acceptor new Connect call TcpServer::newConnection  */
  acceptor_->setNewConnectionCallback(
      std::bind(&TcpServer::newConnection, this, _1, _2));
}

TcpServer::~TcpServer() {
  mainLoop_->assertInLoopThread();
  INFO_("adlServer bye!");
  LOG(INFO) << "TcpServer over" << adl::endl;
  // for (auto &item : connections_) {
  //   TcpConnectionPtr conn(item.second);
  //   item.second.reset();
  //   conn->getLoop()->runInLoop(
  //       std::bind(&TcpConnection::connectDestroyed, conn));
  // }
}

/* 开始跑
  EventLoopThrsetThreadNumeadPool开始创建多个EventLoop线程进行跑，
  acceptor开始listen
 */
void TcpServer::start() {
  LOG(INFO) << "adlServer: " << ip_ << ":" << port_;
  /* if !started_ -> started_=true */
  if (!started_.load(std::memory_order_relaxed)) {
    bool expected = false;
    started_.compare_exchange_strong(expected, true);
  }
  /* 启动subThread */
  assert(!threadPool_->started());
  threadPool_->start(threadInitCallback_);
  /* 启动acceptor */
  assert(!acceptor_->listening());
  mainLoop_->runInLoop([this] { acceptor_->listen(); });
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
  LOG(INFO) << "TcpServer::newConnection\n";
  mainLoop_->assertInLoopThread();
  /*根据当前subLoops中连接数量最小的那个subLoop作为
  新连接的ioloop */
  std::shared_ptr<EventLoop> subLoop = threadPool_->getNextLoop();
  InetAddress localAddr(sock::getLocalAddr(sockfd));
  TcpConnectionPtr conn(
      new TcpConnection(subLoop, sockfd, localAddr, peerAddr));
  conn->setConnectionCallback(connectionCallback_);
  conn->setMessageCallback(messageCallback_);
  conn->setWriteCompleteCallback(writeCompleteCallback_);
  conn->setCloseCallback(
      std::bind(&TcpServer::removeConnection, this, _1)); // FIXME: unsafe
  subLoop->addConnect(conn);
  subLoop->runInLoop([conn]() { conn->connectEstablished(); });
}

/* 作为 tcpConnection 的连接断开回调函数，删除一个连接
  有点奇葩：tcpConnection在某个线程执行了closeCallBack
  会在 mainLoop中 tcpServer::removeConnection，
  然后会在 subLoop中执行TcpConnection::connectDestroyed,
  然后 让channel disableAll 和 remove 从EventLoop中删除。
*/
void TcpServer::removeConnection(const TcpConnectionPtr &conn) {
  auto subLoop = conn->getLoop();
  if (subLoop)
    subLoop->runInLoop(
        std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn) {
  auto subLoop = conn->getLoop();
  if (subLoop) {
    subLoop->assertInLoopThread();
    // LOG_INFO << "TcpServer::removeConnectionInLoop [" << name_
    //          << "] - connection " << conn->name();
    // size_t n = connections_.erase(conn->name());
    subLoop->rmConnect(conn);
    subLoop->queueInLoop([conn]() { conn->connectDestroyed(); });
  }
}

} // namespace adl
