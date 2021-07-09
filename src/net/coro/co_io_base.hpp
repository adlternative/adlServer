#ifndef __CO_IO_BASE_H__
#define __CO_IO_BASE_H__

#ifndef __cpp_impl_coroutine
#define __cpp_impl_coroutine true
#endif

// #if __cplusplus < 202002L
// #undef __cplusplus
// #define __cplusplus 202002L
// #endif

#ifdef __cpp_lib_concepts
#undef __cpp_lib_concepts
#define __cpp_lib_concepts 1
#else
#define __cpp_lib_concepts 1
#endif

#include <coroutine>
#include <span>
#include <stdint.h>
#include <sys/socket.h>

using io_buffer_t = std::span<std::byte>;

struct io_base {
  int fd_;
  int errno_;
  int flags_; /* recv | send flag */
  // sockaddr *addr_; 留给 send_to recv_from...
  // socklen_t addrlen_;
};

struct io_recv;
struct io_send;
struct co_epoll;

struct io_work : io_base {
  explicit io_work(co_epoll &io_service);
  // std::coroutine_handle<void> task_{};
  co_epoll &io_service_;
  io_buffer_t buffer_{};

  io_recv &async_recv(int fd, io_buffer_t buffer, int flags);
  io_send &async_send(int fd, io_buffer_t buffer, int flags);
};

#endif // __CO_IO_BASE_H__