#include "co_io_send.hpp"
#include "co_epoll.hpp"
#include <cerrno>
#include <fcntl.h>
#include <sys/epoll.h>

io_send &io_work::async_send(int fd, io_buffer_t buffer, int flags) {
  fd_ = fd;
  flags_ = flags;
  buffer_ = buffer;
  return *reinterpret_cast<io_send *>(this);
}

bool io_send::await_ready() noexcept {
  return !(fcntl(fd_, F_GETFL, 0) & O_NONBLOCK);
}

void io_send::await_suspend(std::coroutine_handle<void> t) noexcept(false) {
  epoll_event ee{.events =
                     EPOLLOUT | EPOLLERR | EPOLLONESHOT | EPOLLRDHUP | EPOLLET,
                 .data = {.ptr = t.address()}};

  io_service_.try_add(fd_, ee);
}

int64_t io_send::await_resume() noexcept {
  auto sz = send(fd_, buffer_.data(), buffer_.size_bytes(), flags_);
  errno_ = sz < 0 ? errno : 0;
  return sz;
}