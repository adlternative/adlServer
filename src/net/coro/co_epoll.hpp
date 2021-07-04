#ifndef __CO_EPOLL_H__
#define __CO_EPOLL_H__

// #include "co_promise_attr.hpp"
#include "epoll_wrap.hpp"
#include <array>
#include <atomic>
#include <fcntl.h>
#include <unistd.h>
#include <unordered_map>
#include <vector>

class co_epoll : public epoll_wrapper {
  std::vector<epoll_event> epoll_events;
  std::atomic<bool> stopped;
  //   std::unordered_map<int, co_connection> fd2conn;

public:
  void operator()() { run(); }
  explicit co_epoll(int wait_ms)
      : wait_ms_(wait_ms), stopped(false), epoll_events(16) {}
  int wait_ms_;
  void run();
  void stop();
};

#endif // __CO_EPOLL_H__