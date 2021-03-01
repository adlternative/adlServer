#ifndef CHANNEL_H
#define CHANNEL_H
#include "../util.h"
#include <boost/noncopyable.hpp>
#include <functional>
#include <memory>
namespace adl {

class EventLoop;
/* 网络通道类 */
typedef std::function<void()> EventCallback;

class Channel : boost::noncopyable {

public:
  enum status {
    INIT,       /* 初始化 */
    LISTEN,     /* 监听中 */
    DELETED,    /* 已经从EventLoop的Map中删除 */
    NOT_LISTEN, /* 不在监听中 */
  };

  Channel(std::shared_ptr<EventLoop> loop, int fd);
  ~Channel();

  void setReadCallback(EventCallback cb) { readCallback_ = std::move(cb); }
  void setWriteCallback(EventCallback cb) { writeCallback_ = std::move(cb); }
  void setCloseCallback(EventCallback cb) { closeCallback_ = std::move(cb); }
  void setErrorCallback(EventCallback cb) { errorCallback_ = std::move(cb); }

  void enableReading() {
    events_ |= kReadEvent;
    update();
  }
  void disableReading() {
    events_ &= ~kReadEvent;
    update();
  }
  void enableWriting() {
    events_ |= kWriteEvent;
    update();
  }
  void disableWriting() {
    events_ &= ~kWriteEvent;
    update();
  }
  void disableAll() {
    events_ = kNoneEvent;
    update();
  }
  bool isWriting() const { return events_ & kWriteEvent; } /* 判断我们是否注册了可写事件 */
  bool isReading() const { return events_ & kReadEvent; } /* 判断我们是否注册了可读事件 */
  bool noEvent() const { return events_ == kNoneEvent; } /* 判断我们是否没注册事件 */

  int getFd() const { return fd_; }
  int getEvents() const { return events_; }
  status getStatus() const { return status_; }
  void setStatus(status s) { status_ = s; }

  void setRevents(int revt) { revents_ = revt; }

  void handleEvent();
  void remove(); /* 从EventLoop中删除 */
  void debugEvents();

private:
  void update();
  std::shared_ptr<EventLoop> loop_; /* 从属的事件循环EventLoop对象 */
  const int fd_;                    /* 连接套接字 */
  int events_;                      /* 注册事件 */
  int revents_;                     /* 返回事件 */

  status status_; /* 状态 */

  /*事件的分类  */
  static const int kNoneEvent;
  static const int kReadEvent;
  static const int kWriteEvent;

  /* 各种回调函数 */
  EventCallback readCallback_;
  EventCallback writeCallback_;
  EventCallback closeCallback_;
  EventCallback errorCallback_;
};
} // namespace adl
#endif
