#include "EventLoopThreadPool.h"
using namespace adl;

EventLoopThreadPool::EventLoopThreadPool(std::shared_ptr<EventLoop> &baseLoop)
    : baseLoop_(baseLoop) {}

EventLoopThreadPool::~EventLoopThreadPool() {}

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