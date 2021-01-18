
#ifndef BLOCKINGQUEUE_H
#define BLOCKINGQUEUE_H

#include "noncopyable.h"
#include <cassert>
#include <condition_variable>
#include <deque>
#include <mutex>

namespace adl {

template <typename T> class BlockingQueue : noncopyable {
public:
  BlockingQueue() : m_mutex(), queue_() {}
  void put(const T &x) {
    std::unique_lock<std::mutex> ul(m_mutex);
    queue_.push_back(x);
    notEmpty.notify_one();
  }
  void put(T &&x) {
    std::unique_lock<std::mutex> ul(m_mutex);
    queue_.push_back(std::move(x));
    notEmpty.notify_one();
  }
  T take() {
    std::unique_lock<std::mutex> ul(m_mutex);
    while (queue_.empty()) {
      notEmpty.wait(ul);
    }
    assert(!queue_.empty());
    // notEmpty.wait(ul, [&]() { return !queue_.empty(); });
    T t = queue_.front();
    queue_.pop_front();
    return t;
  }
  size_t size() const {
    std::unique_lock<std::mutex> ul(m_mutex);
    return queue_.size();
  }

private:
  mutable std::mutex m_mutex;
  std::condition_variable notEmpty;
  std::deque<T> queue_;
};

} // namespace adl
#endif
