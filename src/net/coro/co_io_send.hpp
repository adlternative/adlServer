#ifndef __CO_IO_SEND_H__
#define __CO_IO_SEND_H__

// #include "co_epoll.hpp"
#include "co_io_base.hpp"

struct io_send : io_work {
  bool await_ready() noexcept;
  void await_suspend(std::coroutine_handle<void> t) noexcept(false);
  int64_t await_resume() noexcept;
};
#endif // __CO_IO_SEND_H__