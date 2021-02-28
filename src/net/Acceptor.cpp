#include "Acceptor.h"
#include "../headFile.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include <fcntl.h>

namespace adl {
Acceptor::Acceptor(const std::shared_ptr<EventLoop> &loop,
                   const InetAddress &listenAddr)
    : loop_(loop),
      acceptSocket_(sock::createNonblockingOrDie(listenAddr.family())),
      acceptChannel_(loop, acceptSocket_.fd()), listening_(false),
      idleFd_(::open("/dev/null", O_RDONLY | O_CLOEXEC)) {
  assert(idleFd_ >= 0);
  acceptSocket_.setReuseAddr(true);
  acceptSocket_.setReusePort(true);
  acceptSocket_.bindAddress(listenAddr);
  acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor() {
  acceptChannel_.disableAll();
  acceptChannel_.remove();
  ::close(idleFd_);
}

void Acceptor::listen() {
  loop_->assertInLoopThread();
  listening_ = true;
  acceptSocket_.listen();
  acceptChannel_.enableReading();
}

void Acceptor::handleRead() {
  int connfd = -1;
  InetAddress peerAddr;

  loop_->assertInLoopThread();
  /*Accept 惊群效应
  Linux 2.6 版本之前，监听同一个 socket 的进程会挂在同一个等待队列上，
      当请求到来时，会唤醒所有等待的进程。
  Linux 2.6 版本之后，通过引入一个标记位
  WQ_FLAG_EXCLUSIVE，只会唤醒对应的等待的线程，解决掉了 accept 惊群效应。
  */
  while (1) {
    connfd = acceptSocket_.accept(&peerAddr);
    if (connfd >= 0) {
      std::string hostport = peerAddr.toIpPort();
      LOG(TRACE) << "Accepts of " << hostport << adl::endl;
      if (newConnectionCallback_) {
        newConnectionCallback_(connfd, peerAddr);
      } else {
        sock::close(connfd);
      }
    } else {
      /* too many file */
      if (errno == EMFILE) {
        /* TOO MANY ONLY FILE 的解决方法 */
        ::close(idleFd_); /* 首先关了一个空闲fd */
        idleFd_ = ::accept(acceptSocket_.fd(), NULL, NULL); /*
        我们就可以用之前关闭的文件描述符去accept这个请求 */
        ::close(idleFd_); /* 然后拒绝服务：因为我们已经没有多余的文件描述符了 */
        idleFd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
        /*再打开空闲fd提供给下次使用 */
      } else if (errno == EINTR) {
        /* 慢系统调用可重启，connect不是 */
        continue;
      } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
        /* 无多余连接 */
        LOG(INFO) << "Acceptor::handleRead over\n";
        break;
      } else {
        LOG(ERROR) << "Acceptor::handleRead accept error\n";
      }
    }
  }
}
} // namespace adl