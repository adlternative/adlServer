#include "EventLoopThreadPool.h"
#include "../util.h"
using namespace adl;

EventLoopThreadPool::EventLoopThreadPool(
    const std::shared_ptr<EventLoop> &baseLoop)
    : baseLoop_(baseLoop) {}

std::shared_ptr<EventLoop> EventLoopThreadPool::getNextLoop() {
  baseLoop_->assertInLoopThread();
  assert(started_);
  std::shared_ptr<EventLoop> loop = baseLoop_;
  if (!loops_.empty()) {
    // round-robin
    loop = loops_[next_];
    ++next_;
    if (implicit_cast<size_t>(next_) >= loops_.size()) {
      next_ = 0;
    }
  }
  return loop;
}

std::vector<std::shared_ptr<EventLoop>> EventLoopThreadPool::getAllLoops() {
  return loops_;
}

void EventLoopThreadPool::start(const ThreadInitCallback &cb) {
  assert(!started_);
  baseLoop_->assertInLoopThread();
  started_ = true;
  for (int i = 0; i < numThreads_; ++i) {
    auto t = std::make_unique<EventLoopThread>(cb);
    threads_.push_back(std::move(t)); /* 添加到EventLoopThread列表 */
    loops_.push_back(t->getLoop());   /* 添加到EventLoop列表 */
  }
  /* assert numThreads_!=0 */
}