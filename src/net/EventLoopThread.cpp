#include "EventLoopThread.h"
using namespace adl;

EventLoopThread::~EventLoopThread() {
  exiting_ = true;
  if (loop_) {
    loop_->quit();
    thread_.join();
  }
}

void EventLoopThread::threadFunc() {
  if (callback_)
    callback_(&*loop_);

  {
    std::unique_lock<std::mutex> lock(mutex_);
    loop_ = std::make_shared<EventLoop>();
    loop_->init();
    cond_.notify_one();
  }
  loop_->loop();
  std::unique_lock<std::mutex> lock(mutex_);
  loop_.reset();
}

EventLoopThread::EventLoopThread(const ThreadInitCallback &cb)
    : loop_(nullptr), thread_(std::bind(&EventLoopThread::threadFunc, this)),
      callback_(cb), exiting_(false) {
  if (!loop_) {
    std::unique_lock<std::mutex> lock(mutex_);
    cond_.wait(lock, [&]() { return loop_; });
  }
}