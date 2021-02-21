#include "Acceptor.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include <fcntl.h>
namespace adl {
Acceptor::Acceptor(EventLoop *loop, const InetAddress &listenAddr)
    : acceptSocket_(sock::createNonblockingOrDie(listenAddr.family())),
      acceptChannel_(loop, acceptSocket_.fd()), listening_(false),
      idleFd_(::open("/dev/null", O_RDONLY | O_CLOEXEC)) {
  acceptSocket_.setReuseAddr(true);
  acceptSocket_.setReusePort(true);
  acceptSocket_.bindAddress(listenAddr);
  acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead, this));
}
Acceptor::~Acceptor() {
  acceptChannel_.disableAll();
  acceptChannel_.remove();
}

void Acceptor::listen() {
  loop_->assertInLoopThread();
  listening_ = true;
  acceptSocket_.listen();
  acceptChannel_.enableReading();
}

void Acceptor::handleRead() {
  loop_->assertInLoopThread();
  InetAddress peerAddr;
  // FIXME loop until no more
  int connfd = acceptSocket_.accept(&peerAddr);
  if (connfd >= 0) {
    // std::string hostport = peerAddr.toIpPort();
    // LOG_TRACE << "Accepts of " << hostport;
    if (newConnectionCallback_) {
      newConnectionCallback_(connfd, peerAddr);
    } else {
      sock::close(connfd);
    }
  } else {
    // LOG_SYSERR << "in Acceptor::handleRead";
    // Read the section named "The special problem of
    // accept()ing when you can't" in libev's doc.
    // By Marc Lehmann, author of libev.
    if (errno == EMFILE) /* too many file */
    {
      /* TOO MANY ONLY FILE 的解决方法 */
      ::close(idleFd_); /* 首先关了一个空闲fd */
      idleFd_ = ::accept(acceptSocket_.fd(), NULL, NULL); /*
       我们就可以accept这个请求 */
      ::close(idleFd_); /* 然后关闭了它 */
      idleFd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC); /*
       再打开空闲fd提供给下次使用 */
    }
  }
}
} // namespace adl