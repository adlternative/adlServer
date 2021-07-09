#include "co_epoll.hpp"

#ifndef __cpp_impl_coroutine
#define __cpp_impl_coroutine true
#endif
#include <coroutine>

void co_epoll::run() {
  int cnt;

  while (!stopped) {
    try {
      cnt = loop(epoll_events_.data(), epoll_events_.size(), wait_ms_);
    } catch (...) {
    }
    for (auto i = 0; i != cnt; i++) {
      try {
        auto &ee = epoll_events_[i];
        /* 连接指针 */
        // coroutine_handle<>::from_address(ee.data.ptr);
        auto co = std::coroutine_handle<void>::from_address(ee.data.ptr);

        if (ee.events & EPOLLIN) {
            co.resume();
        }
        if (ee.events & EPOLLOUT) {
            co.resume();
        }
        if (ee.events & EPOLLRDHUP) {
          /* 回收资源，关闭连接 */
        }
      } catch (...) {
      }
    }

    if (cnt >= epoll_events_.size()) {
      epoll_events_.resize(cnt * 2);
    }
    if (cnt <= epoll_events_.size() / 5) {
      epoll_events_.resize(cnt / 2);
    }
  }
}

void co_epoll::stop() { stopped = true; }
