#ifndef EVENTLOOP_H
#define EVENTLOOP_H
#include "../base/currentThread.h"
#include "../log/timeStamp.h"
#include "Epoller.h"
#include "tcpConnection.h"
#include <boost/noncopyable.hpp>
#include <functional>
#include <memory>
#include <mutex>
#include <unordered_set>
#include <vector>
namespace adl {
class Channel;
class Epoller;
class EventLoop : boost::noncopyable,
                  public std::enable_shared_from_this<EventLoop> {
public:
  using Functor = std::function<void()>;
  EventLoop(bool mainLoop = false);
  ~EventLoop();
  void init();
  void loop(); /* 循环 epoll_wait */
  void quit(); /* 退出 */
  void runInLoop(Functor cb);
  void queueInLoop(Functor cb);

  void wakeup();
  void updateChannel(Channel *channel);
  void removeChannel(Channel *channel);
  bool hasChannel(Channel *channel);

  void addConnect(std::shared_ptr<TcpConnection> con);
  void rmConnect(std::shared_ptr<TcpConnection> con);
  void assertInLoopThread();
  bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }

private:
  void handleRead(); //用于 读取Wakeupfd 为了唤醒所发送的内容
  void doPendingFunctors(); /* 执行小任务函数 */

  bool isMainLoop() { return isMainLoop_; }
  bool isMainLoop_;                          /* 是否是主Loop */
  std::atomic<bool> quit_;                   /* 是否退出 */
  std::atomic<bool> eventHandling_;          /* 是否在处理事件 */
  std::atomic<bool> callingPendingFunctors_; /* 是否在处理小任务 */

  const pid_t threadId_; /* EventLoop”所属“的IO线程的线程号 */
  int wakeupFd_;         /* 用于唤醒EventLoop的wakeupFd_ */
  std::unique_ptr<Channel> wakeupChannel_; /* 用于唤醒EventLoop的通道 */
  std::unique_ptr<Epoller> poller_;        /* poller(epoll封装类) */
  mutable std::mutex mutex_;      /* 用于小任务的多线程同步 */
  timeStamp pollReturnTime_;      /* epoll_wait 返回的时间 */
  Channel *currentActiveChannel_; /* 当前在处理的通道 */
  using ChannelList = std::vector<Channel *>;
  ChannelList activeChannels_; /* 活跃通道列表 */
  using FunctorList = std::vector<Functor>;
  std::vector<Functor> pendingFunctors_; /* 小任务列表 */
  using TcpConnectSet = std::unordered_set<std::shared_ptr<TcpConnection>>;
  TcpConnectSet connectSet_; /* 连接集合 */

  // TimerId runAt(Timestamp time, TimerCallback cb);
  // TimerId runAfter(double delay, TimerCallback cb);
  // TimerId runEvery(double interval, TimerCallback cb);
  // void cancel(TimerId timerId);
};
} // namespace adl
#endif
