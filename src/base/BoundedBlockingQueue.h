
#ifndef BOUNDEDBLOCKINGQUEUE_H
#define BOUNDEDBLOCKINGQUEUE_H

#include "noncopyable.h"
#include <assert.h>
#include <boost/circular_buffer.hpp>
#include <condition_variable>
#include <functional>
#include <mutex>

namespace adl {

template <typename T> class BoundedBlockingQueue : noncopyable {
public:
  explicit BoundedBlockingQueue(int maxSize) : queue(maxSize) {}
  void put(T &&t) {
    std::unique_lock<std::mutex> ul(mutex_);
    while (queue.full()) {
      notFull.wait(ul);
    }
    assert(!queue.full());
    queue.push_back(std::move(t));
    notEmpty.notify_one();
  }
  void put(T &t) {

    std::unique_lock<std::mutex> ul(mutex_);
    while (queue.full()) {
      notFull.wait(ul);
    }
    assert(!queue.full());
    queue.push_back(t);
    notEmpty.notify_one();
  }
  T take() {
    std::unique_lock<std::mutex> ul(mutex_);
    while (queue.empty()) {
      notEmpty.wait(ul);
    }
    assert(!queue.empty());
    T t(std::move(queue.front()));
    queue.pop_front();
    notFull.notify_one();
    return t;
  }
  bool empty() {
    std::unique_lock<std::mutex> ul(mutex_);
    return queue.empty();
  }
  bool full() {
    std::unique_lock<std::mutex> ul(mutex_);
    return queue.full();
  }
  bool size() {
    std::unique_lock<std::mutex> ul(mutex_);
    return queue.size();
  }
  bool capacity() {
    std::unique_lock<std::mutex> ul(mutex_);
    return queue.capacity();
  }

private:
  mutable std::mutex mutex_;
  std::condition_variable notEmpty;
  std::condition_variable notFull;
  boost::circular_buffer<T> queue;
};

} // namespace adl

#endif
