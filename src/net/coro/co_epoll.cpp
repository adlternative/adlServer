#include "co_epoll.hpp"

#ifndef __cpp_impl_coroutine
#define __cpp_impl_coroutine true
#endif
#include <coroutine>

void co_epoll::run() {
  int cnt;

  while (!stopped) {
    try {
      cnt = loop(epoll_events.data(), epoll_events.size(), wait_ms_);
    } catch (...) {
    }
    for (auto i = 0; i != cnt; i++) {
      try {
        auto &ee = epoll_events[i];
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

    if (cnt >= epoll_events.size()) {
      epoll_events.resize(cnt * 2);
    }
    if (cnt <= epoll_events.size() / 5) {
      epoll_events.resize(cnt / 2);
    }
  }
}

void co_epoll::stop() { stopped = true; }
