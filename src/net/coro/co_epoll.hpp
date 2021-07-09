#ifndef __CO_EPOLL_H__
#define __CO_EPOLL_H__

#include "../../base/noncopyable.hpp"
// #include "connection.hpp"
#include "epoll_wrap.hpp"
#include <array>
#include <atomic>
#include <fcntl.h>
#include <memory>
#include <unistd.h>
#include <unordered_map>
#include <vector>
class co_epoll : public epoll_wrapper, adl::noncopyable {
  std::vector<struct epoll_event> epoll_events_;
  std::atomic<bool> stopped;
  // std::unordered_map<int, std::weak_ptr<adl::Connection>> connect_map;

public:
  void operator()() { run(); }
  explicit co_epoll(int wait_ms)
      : wait_ms_(wait_ms), stopped(false), epoll_events_(16) {}
  int wait_ms_;
  void run();
  void stop();
};

#endif // __CO_EPOLL_H__