#ifndef EVENTLOOPTHREAD_H
#define EVENTLOOPTHREAD_H
#include "EventLoop.h"
#include <boost/noncopyable.hpp>

namespace adl {

class EventLoopThread : boost::noncopyable {
public:
  using ThreadInitCallback = std::function<void(const std::shared_ptr<EventLoop>&)>;
  EventLoopThread(const ThreadInitCallback &cb = ThreadInitCallback());
  ~EventLoopThread();
  std::shared_ptr<EventLoop> getLoop() { return loop_; }

private:
  void threadFunc();
  std::shared_ptr<EventLoop> loop_;
  std::thread thread_;
  std::atomic<bool> exiting_;
  std::mutex mutex_;
  std::condition_variable cond_;
  ThreadInitCallback callback_;
};

} // namespace adl
#endif
