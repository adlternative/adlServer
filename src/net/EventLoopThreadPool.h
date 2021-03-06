#ifndef EVENTLOOPTHREADPOOL_H
#define EVENTLOOPTHREADPOOL_H
#include "EventLoopThread.h"
#include <boost/noncopyable.hpp>
#include <cassert>
#include <functional>
#include <memory>
#include <vector>
namespace adl {

class EventLoopThreadPool : boost::noncopyable {
public:
  using ThreadInitCallback = EventLoopThread::ThreadInitCallback;
  EventLoopThreadPool(const std::shared_ptr<EventLoop> &baseLoop);
  ~EventLoopThreadPool() = default;
  std::shared_ptr<EventLoop> getNextLoop();
  // EventLoop* getLoopForHash(size_t hashCode);
  std::vector<std::shared_ptr<EventLoop>> getAllLoops();

  void setThreadNum(int numThreads) { numThreads_ = numThreads; }
  void start(const ThreadInitCallback &cb = ThreadInitCallback());
  bool started() const { return started_; }

private:
  int numThreads_;                      /* 线程数 */
  std::shared_ptr<EventLoop> baseLoop_; /* mainloop */
  std::atomic<bool> started_;           /* 开始了么 */
  int next_;                            /* 轮询的下一个的坐标 */
  std::vector<std::unique_ptr<EventLoopThread>> threads_;
  std::vector<std::shared_ptr<EventLoop>> loops_;
};

} // namespace adl
#endif
