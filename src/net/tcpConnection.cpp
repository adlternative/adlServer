#include "tcpConnection.h"
#include "Channel.h"
#include "EventLoop.h"
namespace adl {

/* 默认的消息回调 */
void defaultMessageCallback(const TcpConnectionPtr &, netBuffer *buf) {
  buf->reset();
}
/* 默认的连接回调 */
void defaultConnectionCallback(const TcpConnectionPtr &conn) {
  // LOG_DEBUG << (conn->connected() ? "up" : "down");
}

TcpConnection::TcpConnection(const std::shared_ptr<EventLoop> &loop, int sockfd,
                             const InetAddress &localAddr,
                             const InetAddress &peerAddr)
    : loop_(loop), state_(kConnecting), socket_(new Socket(sockfd)),
      channel_(new Channel(loop_, sockfd)), localAddr_(localAddr),
      peerAddr_(peerAddr) {

  channel_->setReadCallback(std::bind(&TcpConnection::handleRead,
                                      this /* , std::placeholders::_1 */));
  channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
  channel_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
  channel_->setErrorCallback(std::bind(&TcpConnection::handleError, this));
  /* log */
  socket_->setKeepAlive(true);
}

TcpConnection::~TcpConnection() {
  /* log */
  assert(state_ == kDisconnected);
}

void TcpConnection::send(const void *message, int len) {
  if (state_ == kConnected) {
    void (TcpConnection::*fp)(const void *message, size_t len) =
        &TcpConnection::sendInLoop;

    loop_->runInLoop(std::bind(fp, this, message, len));
  }
}

void TcpConnection::shutdown() {
  if (state_ == kConnected) {
    setState(kDisconnecting);
    // FIXME: shared_from_this()?
    loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
  }
}

void TcpConnection::forceClose() {
  // FIXME: use compare and swap
  if (state_ == kConnected || state_ == kDisconnecting) {
    setState(kDisconnecting);
    loop_->queueInLoop(
        std::bind(&TcpConnection::forceCloseInLoop, shared_from_this()));
  }
}

void TcpConnection::forceCloseInLoop() {
  loop_->assertInLoopThread();
  if (state_ == kConnected || state_ == kDisconnecting) {
    // as if we received 0 byte in handleRead();
    handleClose();
  }
}
/* 处理读事件:ok */
void TcpConnection::handleRead(/* timeStamp receiveTime */) {
  loop_->assertInLoopThread();
  int savedErrno;
  bool closed = false;
  /* read to inputBuf*/
  int n = inputBuffer_.readFd(socket_->fd(), &savedErrno, &closed);
  if (closed) {
    /* 如果对端关闭，我们调用关闭回调 */
    handleClose();
  } else if (n > 0) {
    /* 如果读到了数据，我们调用消息回调函数 */
    messageCallback_(shared_from_this(), &inputBuffer_);
  } else { /* 出错 */
    /* 调用错误回调 */
    errno = savedErrno;
    // LOG_SYSERR << "TcpConnection::handleRead";
    handleError();
  }
}

/* 处理写事件:ok */
void TcpConnection::handleWrite() {
  loop_->assertInLoopThread();
  /* 我们的确是在监听可写事件 */
  if (channel_->isWriting()) {
    ssize_t n = sock::write(channel_->getFd(), outputBuffer_.peek(),
                            outputBuffer_.readable());
    if (n > 0) {
      /* 我们outputBUf中可读的缓冲数量减少n */
      outputBuffer_.retrieve(n);
      /* 如果已经输出缓冲区全部发送完成 */
      if (!outputBuffer_.readable()) {
        /* 我们取消监听可写事件 */
        channel_->disableWriting();
        /* 触发写完成事件 */
        if (writeCompleteCallback_) {
          loop_->queueInLoop(
              std::bind(writeCompleteCallback_, shared_from_this()));
        }
        /* 如果已经是关闭连接状态，关闭写端 */
        if (state_ == kDisconnecting) {
          shutdownInLoop();
        }
      }
    } else {
      /* 写发生了错误 */
      // LOG
    }
  } else {
    /* 否则现在并不在监听写事件 ，啥事不做? LOG*/
  }
}

/* 处理关闭连接事件
  QUE::muduo原来比较奇怪，
  用了一个新的指针去执行
  connectionCallback_和closeCallback_
*/
void TcpConnection::handleClose() {
  loop_->assertInLoopThread();
  assert(state_ == kConnected || state_ == kDisconnecting);
  setState(kDisconnected);
  /* LOG */
  channel_->disableAll();

  closeCallback_(shared_from_this());
}

/* 处理错误:ok
  出错的时候通过getSocketError获得套接字上的错误，
  并将其打印到日志文件中 */
void TcpConnection::handleError() {
  int err = sock::getSocketError(channel_->getFd());
  // LOG()
}

/* 在IO loop中发送信息 */
void TcpConnection::sendInLoop(const void *message, size_t len) {
  loop_->assertInLoopThread();
  ssize_t nwrote = 0;
  size_t remaining = len;
  bool faultError = false;
  if (state_ == kDisconnected) {
    // LOG_WARN << "disconnected, give up writing";
    return;
  }
  /* 之前没有监听写事件 ，空的输出缓冲区，表示os的缓冲区没有满
    我们就可以直接write我们本次想发送的message  */
  if (!channel_->isWriting() && outputBuffer_.readable() == 0) {
    nwrote = sock::write(channel_->getFd(), message, len);
    if (nwrote >= 0) {
      /* 写一次，本次信息全写完触发一次写完成事件 */
      remaining = len - nwrote;
      if (remaining == 0 && writeCompleteCallback_) {
        loop_->queueInLoop(
            std::bind(writeCompleteCallback_, shared_from_this()));
      }
    } else { /* 出错 */
      nwrote = 0;
      /* OS写缓冲区满 */
      if (errno != EWOULDBLOCK && errno != EAGAIN) {
        // LOG_SYSERR << "TcpConnection::sendInLoop";
        if (errno == EPIPE || errno == ECONNRESET) // FIXME: any others?
        {
          /* 致命错误 */
          faultError = true;
        }
      }
    }
  }
  assert(remaining <= len);
  if (!faultError && remaining > 0) {
    /* 未发送的数据添加到outputbuffer中  */
    outputBuffer_.append(static_cast<const char *>(message) + nwrote,
                         remaining);
    // 关注epollout事件，OS写缓冲区不满，会触发HandleWrite
    if (!channel_->isWriting())
      channel_->enableWriting();
  }
}

void TcpConnection::sendInLoop(const char *message, size_t len) {
  return sendInLoop(static_cast<const void *>(message), len);
}

void TcpConnection::sendInLoop(string &&message) {
  return sendInLoop(static_cast<const void *>(std::move(message).c_str()),
                    std::move(message).size());
}

void TcpConnection::shutdownInLoop() {

  loop_->assertInLoopThread();
  if (!channel_->isWriting()) {
    // we are not writing
    socket_->shutdownWrite();
  }
}

/* 将连接套接字禁用Nagle算法 */
void TcpConnection::setTcpNoDelay(bool on) { socket_->setTcpNoDelay(on); }

/* 开始监听套接字读事件 */
void TcpConnection::startRead() {
  loop_->runInLoop(std::bind(&TcpConnection::startReadInLoop, this));
}

/* 开始监听套接字读事件 IN ioLoop */
void TcpConnection::startReadInLoop() {
  loop_->assertInLoopThread();
  if (!reading_ || !channel_->isReading()) {
    channel_->enableReading();
    reading_ = true;
  }
}

/* 停止监听套接字读事件 */
void TcpConnection::stopRead() {
  loop_->runInLoop(std::bind(&TcpConnection::stopReadInLoop, this));
}

/* 停止监听套接字读事件 IN ioLoop */
void TcpConnection::stopReadInLoop() {
  loop_->assertInLoopThread();
  if (reading_ || channel_->isReading()) {
    channel_->disableReading();
    reading_ = false;
  }
}

/* 连接建立会调用connectionCallback_ */
void TcpConnection::connectEstablished() {
  loop_->assertInLoopThread();
  assert(state_ == kConnecting);
  setState(kConnected);

  channel_->enableReading();

  connectionCallback_(shared_from_this());
}

/* 连接断开也会调用connectionCallback_ */
void TcpConnection::connectDestroyed() {
  loop_->assertInLoopThread();
  if (state_ == kConnected) {
    setState(kDisconnected);
    channel_->disableAll();

    connectionCallback_(shared_from_this());
  }
  channel_->remove();
}

} // namespace adl