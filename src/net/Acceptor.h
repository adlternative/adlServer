#ifndef ACCEPTOR_H
#define ACCEPTOR_H
#include "Channel.h"
#include "Socket.h"
#include <atomic>
#include <boost/noncopyable.hpp>
namespace adl {
class EventLoop;
class InetAddress;
class Acceptor : boost::noncopyable {
public:
  using NewConnectionCallback =
      std::function<void(int sockfd, const InetAddress &)>;

  Acceptor(const std::shared_ptr<EventLoop> &loop,
           const InetAddress &listenAddr);
  ~Acceptor();

  /* 设置新连接回调函数  */
  void setNewConnectionCallback(const NewConnectionCallback &cb) {
    newConnectionCallback_ = cb;
  }

  void listen();
  bool listening() const { return listening_; }

private:
  void handleRead();
  std::shared_ptr<EventLoop> loop_; /* main loop */
  Socket acceptSocket_;
  Channel acceptChannel_;
  std::atomic<bool> listening_;                 /* 是否开始监听 */
  NewConnectionCallback newConnectionCallback_; /* 建立连接时的回调函数 */
  int idleFd_; /* 用来解决 too many files ，用来关闭连接*/
};

} // namespace adl
#endif
