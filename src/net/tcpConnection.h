#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H
#include "../base/timeStamp.h"
#include "InetAddress.h"
#include "callBack.h"
#include "netBuffer.h"
#include <boost/noncopyable.hpp>
// #include <string_view>
namespace adl {

class Channel;
class EventLoop;
class Socket;
using std::string;

class TcpConnection : boost::noncopyable,
                      public std::enable_shared_from_this<TcpConnection> {
public:
  TcpConnection(const std::weak_ptr<EventLoop> &loop, int sockfd,
                const InetAddress &localAddr, const InetAddress &peerAddr);
  ~TcpConnection();

  std::shared_ptr<EventLoop> getLoop() const { return loop_.lock(); }
  const InetAddress &localAddress() const { return localAddr_; }
  const InetAddress &peerAddress() const { return peerAddr_; }
  bool connected() const { return state_ == kConnected; }
  bool disconnected() const { return state_ == kDisconnected; }
  // void send(string&& message); // C++11
  void send(const void *message, int len);
  void send(const string &message);
  // void send(Buffer&& message); // C++11

  void send(netBuffer *message); // this one will swap data
  void shutdown();               // NOT thread safe, no simultaneous calling
  // void shutdownAndForceCloseAfter(double seconds); // NOT thread safe, no
  // simultaneous calling
  void forceClose();
  void forceCloseWithDelay(double seconds);
  void setTcpNoDelay(bool on);
  // reading or not
  void startRead();
  void stopRead();
  bool isReading() const {
    return reading_;
  }; // NOT thread safe, may race with start/stopReadInLoop

  void setConnectionCallback(const ConnectionCallback &cb) {
    connectionCallback_ = cb;
  }

  void setMessageCallback(const MessageCallback &cb) { messageCallback_ = cb; }

  void setWriteCompleteCallback(const WriteCompleteCallback &cb) {
    writeCompleteCallback_ = cb;
  }

  netBuffer *inputBuffer() { return &inputBuffer_; }

  netBuffer *outputBuffer() { return &outputBuffer_; }

  /// Internal use only.
  void setCloseCallback(const CloseCallback &cb) { closeCallback_ = cb; }

  // called when TcpServer accepts a new connection
  void connectEstablished(); // should be called only once
  // called when TcpServer has removed me from its map
  void connectDestroyed(); // should be called only once

  enum StateE { kDisconnected, kConnecting, kConnected, kDisconnecting };

private:
  void handleRead(/* timeStamp receiveTime */);
  void handleWrite();
  void handleClose();
  void handleError();
  void sendInLoop(string &&message);
  // void sendInLoop(const StringPiece &message);
  void sendInLoop(const void *message, size_t len);
  void sendInLoop(const char *message, size_t len);

  void shutdownWriteInLoop();/* 关闭写 */
  // void shutdownAndForceCloseInLoop(double seconds);
  void forceCloseInLoop();
  void setState(StateE s) { state_ = s; }
  const char *stateToString() const;
  void startReadInLoop();
  void stopReadInLoop();

  std::weak_ptr<EventLoop> loop_;
  const string name_;
  StateE state_; // FIXME: use atomic variable
  bool reading_;
  // we don't expose those classes to client.
  std::unique_ptr<Socket> socket_;
  std::unique_ptr<Channel> channel_;
  const InetAddress localAddr_;
  const InetAddress peerAddr_;
  ConnectionCallback connectionCallback_;
  MessageCallback messageCallback_;
  WriteCompleteCallback writeCompleteCallback_;
  // HighWaterMarkCallback highWaterMarkCallback_;
  CloseCallback closeCallback_;
  size_t highWaterMark_;
  netBuffer inputBuffer_;
  netBuffer outputBuffer_; // FIXME: use list<Buffer> as output buffer.
  // FIXME: creationTime_, lastReceiveTime_
  //        bytesReceived_, bytesSent_
};

typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;

void defaultMessageCallback(const TcpConnectionPtr &, netBuffer *buf);
void defaultConnectionCallback(const TcpConnectionPtr &conn);

} // namespace adl
#endif
