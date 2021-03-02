#include "Channel.h"
#include "../tool/usage.h"
#include "EventLoop.h"
#include <poll.h>
#include <sys/epoll.h>
using namespace adl;
/* EPOLLWAKEUP */
const int Channel::kNoneEvent = EPOLLET;                         /* 无事件 */
const int Channel::kReadEvent = EPOLLIN | EPOLLRDHUP | EPOLLPRI; /* 读事件  */
const int Channel::kWriteEvent = EPOLLOUT; /* 写事件 OUT */

Channel::Channel(std::shared_ptr<EventLoop> loop, int fd)
    : fd_(fd), events_(kNoneEvent), loop_(loop), revents_(0), status_(INIT) {}

Channel::~Channel() {}

void Channel::update() {
  // addedToLoop_ = true;
  loop_->updateChannel(this);
}

void Channel::handleEvent() {

  /* EPOLLHUP 貌似本端错误导致的关闭 需要进一步查证 */
  if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)) {
    /* EPOLLHUP: 设备已断开连接，
    或者管道或FIFO已被打开以进行写入的最后一个进程关闭。
    设置后，FIFO的挂起状态将一直持续到某个进程打开FIFO进行写入
    或关闭FIFO的所有只读文件描述符为止。
    此事件和POLLOUT是互斥的；如果发生挂断，流将永远不可写。
    但是，此事件与POLLIN，POLLRDNORM，POLLRDBAND或POLLPRI并不互斥。
    该标志仅在revents位掩码中有效。在事件成员中应将其忽略。 */

    /* EPOLLHUP 对端断开可写到日志中 */
    // if (logHup_)
    // {
    //   LOG_WARN << "fd = " << fd_ << " Channel::handle_event() EPOLLHUP";
    // }
    if (closeCallback_)
      closeCallback_();
  }
  /* Invalid polling request 打印日志
  POLLNVAL如果关闭文件描述符，然后尝试从关闭的fd中读取，将触发该程序。
 */
  /* 处理错误事件 */
  if (revents_ & EPOLLERR) {
    if (errorCallback_)
      errorCallback_();
  }
  /* 处理读事件       包括可能的连接事件，关闭连接，带外数据 */
  if (revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
    if (readCallback_)
      readCallback_();
  }
  /* 处理写事件 */
  if (revents_ & EPOLLOUT) {
    if (writeCallback_)
      writeCallback_();
  }
}

void Channel::remove() {
  // addedToLoop_ = false;
  loop_->removeChannel(this);
}

void Channel::debugEvents() {
  if (events_ & EPOLLIN) {
    ERR_("EPOLLIN");
  }
  if (events_ & EPOLLOUT) {
    ERR_("EPOLLOUT");
  }
  if (events_ & EPOLLET) {
    ERR_("EPOLLET");
  }
}

/*
 *EPOLLRDHUP: Stream socket peerclosed connection,
 * or shut down writing half of connection. */

/*
 *如果用 epoll , 此时不会收到 EPOLLHUP ，而会收到 EPOLLRDHUP (要额外关注)
 *完全关闭则会收到 EPOLLHUP 。
 *不可写（写关闭）需要在 write 后发现 EPIPE 错误方可确认。
 */

/* 
云风最近的一篇博客讲了半关闭问题
https://blog.codingnow.com/2021/02/skynet_tcp_halfclose.html#more

 */