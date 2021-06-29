#ifndef __cpp_impl_coroutine
#define __cpp_impl_coroutine true
#endif

#if __cplusplus < 202002L
#undef __cplusplus
#define __cplusplus 202002L
#endif

#include "epoll_wrap.hpp"
#include <array>
#include <coroutine>
#include <fcntl.h>
#include <unistd.h>
#include <vector>

struct co_epoll_task {
  struct epoll_event events_;
  int fd_;
  co_epoll_task() : fd_(0) {}
  co_epoll_task(int fd, std::coroutine_handle<> &ptr) : fd_(fd) {
    events_.data.ptr = ptr.address();
    events_.data.fd = fd;
  }
  ~co_epoll_task() {}
};

class co_epoll : epoll_wrapper {
  std::vector<epoll_event> epoll_events;

public:
  co_epoll() : epoll_wrapper() {}
  int loop(int wait_ms = -1) {
    int i;
    int cnt;

    while (1) {
      try {
        cnt = epoll_wrapper::loop(epoll_events.data(), epoll_events.size(),
                                  wait_ms);
      } catch (...) {
      }
      for (i = 0; i != cnt; i++) {
        try {

          if (auto coro = std::coroutine_handle<>::from_address(
                  epoll_events[i].data.ptr)) {
            if (epoll_events[i].events & EPOLLIN) {
              int ret;
              for (;;) {
                std::array<char, 4096> buf;
                ret = read(epoll_events[i].data.fd, buf.data(), 4096);
                if (ret == 0) {
                  /* close */
                }
                if (ret == -1) {
                  if (errno == EINTR)
                    continue;
                  /* 本次读干净了，就resume */
                  if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    coro.resume();
                    break;
                  }
                }
              }
            }
          }

          /* 恢复协程 */
          // coro.resume();
        } catch (...) {
        }
      }
      if (cnt == epoll_events.size())
        epoll_events.resize(cnt * 2);

      if (cnt <= epoll_events.size() / 5)
        epoll_events.resize(cnt / 2);
    }
  }
  void add(co_epoll_task &t, int opt) {
    t.events_.events |= opt;
    epoll_wrapper::add(t.fd_, t.events_);
  }
  void del(co_epoll_task &t) { epoll_wrapper::del(t.fd_); }
  void mod(co_epoll_task &t, int opt) {
    t.events_.events |= opt;
    epoll_wrapper::mod(t.fd_, t.events_);
  }
};