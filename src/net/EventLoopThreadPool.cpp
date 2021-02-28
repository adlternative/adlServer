#include "EventLoopThreadPool.h"
#include "../headFile.h"
using namespace adl;

EventLoopThreadPool::EventLoopThreadPool(
    const std::shared_ptr<EventLoop> &baseLoop)
    : baseLoop_(baseLoop) {}

/* 简单的的负载均衡：
根据当前subLoops中连接数量最小的那个subLoop作为
新连接的ioloop */
std::shared_ptr<EventLoop> EventLoopThreadPool::getNextLoop() {
  baseLoop_->assertInLoopThread();
  assert(started_);
  std::shared_ptr<EventLoop> loop = baseLoop_;
  loop = *std::min_element(loops_.begin(), loops_.end(),
                           [](const std::shared_ptr<EventLoop> &lhs,
                              const std::shared_ptr<EventLoop> &rhs) {
                             return lhs->connectSize() < rhs->connectSize();
                           });
  return loop;
}

std::vector<std::shared_ptr<EventLoop>> EventLoopThreadPool::getAllLoops() {
  return loops_;
}

void EventLoopThreadPool::start(const ThreadInitCallback &cb) {
  LOG(INFO) << "EventLoopThreadPool::start" << adl::endl;
  assert(!started_);
  baseLoop_->assertInLoopThread();
  started_ = true;
  for (int i = 0; i < numThreads_; ++i) {
    auto t = std::make_unique<EventLoopThread>(cb);
    loops_.push_back(t->getLoop()); /* 添加到EventLoop列表 */
    /* 添加到EventLoopThread列表 ,注意 std::move 以后unique_ptr不能使用了*/
    threads_.push_back(std::move(t));
  }
  /* assert numThreads_!=0 */
}