#include "co_io_recv.hpp"
#include "co_epoll.hpp"
#include <cerrno>
#include <fcntl.h>
#include <sys/epoll.h>

io_recv &io_work::async_recv(int fd, io_buffer_t buffer, int flags) {
  fd_ = fd;
  flags_ = flags;
  buffer_ = buffer;
  return *reinterpret_cast<io_recv *>(this);
}

bool io_recv::await_ready() noexcept {
  return !(fcntl(fd_, F_GETFL, 0) & O_NONBLOCK);
}

void io_recv::await_suspend(std::coroutine_handle<void> t) noexcept(false) {
  epoll_event ee{.events =
                     EPOLLIN | EPOLLERR | EPOLLONESHOT | EPOLLRDHUP | EPOLLET,
                 .data = {.ptr = t.address()}};

  io_service_.try_add(fd_, ee);
}

int64_t io_recv::await_resume() noexcept {
  auto sz = recv(fd_, buffer_.data(), buffer_.size_bytes(), flags_);
  errno_ = sz < 0 ? errno : 0;
  return sz;
}