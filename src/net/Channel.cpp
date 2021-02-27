#include "Channel.h"
#include "../util.h"
#include "EventLoop.h"
#include <poll.h>
#include <sys/epoll.h>
using namespace adl;
const int Channel::kNoneEvent = EPOLLET;            /* 无事件 */
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI; /* 读事件 IN PRI */
const int Channel::kWriteEvent = EPOLLOUT;          /* 写事件 OUT */

Channel::Channel(std::shared_ptr<EventLoop> loop, int fd)
    : fd_(fd), events_(kNoneEvent), loop_(loop), revents_(0), status_(INIT) {}

Channel::~Channel() {}

void Channel::update() {
  // addedToLoop_ = true;
  loop_->updateChannel(this);
}

void Channel::handleEvent() {
  if ((revents_ & POLLHUP) && !(revents_ & POLLIN)) {
    /* POLLHUP: 设备已断开连接，
    或者管道或FIFO已被打开以进行写入的最后一个进程关闭。
    设置后，FIFO的挂起状态将一直持续到某个进程打开FIFO进行写入
    或关闭FIFO的所有只读文件描述符为止。
    此事件和POLLOUT是互斥的；如果发生挂断，流将永远不可写。
    但是，此事件与POLLIN，POLLRDNORM，POLLRDBAND或POLLPRI并不互斥。
    该标志仅在revents位掩码中有效。在事件成员中应将其忽略。 */

    /* POLLHUP 对端断开可写到日志中 */
    // if (logHup_)
    // {
    //   LOG_WARN << "fd = " << fd_ << " Channel::handle_event() POLLHUP";
    // }
    if (closeCallback_)
      closeCallback_();
  }
  /* Invalid polling request 打印日志
  POLLNVAL如果关闭文件描述符，然后尝试从关闭的fd中读取，将触发该程序。
 */
  if (revents_ & POLLNVAL) {
    // LOG_WARN << "fd = " << fd_ << " Channel::handle_event() POLLNVAL";
  }
  /* 处理错误事件 */
  if (revents_ & (POLLERR | POLLNVAL)) {
    if (errorCallback_)
      errorCallback_();
  }
  /* 处理读事件 */
  if (revents_ & (POLLIN | POLLPRI | POLLRDHUP)) {
    if (readCallback_)
      readCallback_();
  }
  /* 处理写事件 */
  if (revents_ & POLLOUT) {
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
    ERR("EPOLLIN");
  }
  if (events_ & EPOLLOUT) {
    ERR("EPOLLOUT");
  }
  if (events_ & EPOLLET) {
    ERR("EPOLLET");
  }
}
