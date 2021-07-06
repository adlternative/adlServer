#ifndef __SOCKET_H__
#define __SOCKET_H__

#include "../../base/noncopyable.hpp"
#include "socket_wrap.hpp"
#include <memory>
#include <stdexcept>

#define SPDLOG_FMT_EXTERNAL
#include <spdlog/spdlog.h>

namespace adl {

namespace socket {

class Socket : noncopyable {
public:
  Socket() : socket_fd_(-1) {}
  explicit Socket(int socket_fd) : socket_fd_(socket_fd) {}
  Socket(Socket &&s) noexcept : socket_fd_(s.socket_fd_) {}
  Socket &operator=(Socket &&s) noexcept {
    this->socket_fd_ = s.socket_fd_;
    return *this;
  }

  void close() noexcept {
    if (socket_fd_ != -1) [[likely]] {
      int err = 0;
      adl::socket::socket_close(socket_fd_, &err);
      if (err) {
        spdlog::error("socket close error?!");
      }
    }
  }

  ~Socket() { this->close(); }

  template <enum socket_family family, enum socket_protocol protocol>
  static std::unique_ptr<Socket> create_socket(int non_block = false,
                                               int close_exec = false);

  int get_fd() noexcept { return socket_fd_; }

private:
  int socket_fd_;
};

template <enum socket_family family, enum socket_protocol protocol>
std::unique_ptr<Socket> Socket::create_socket(int non_block, int close_exec) {
  int err;

  auto fd = socket_init(family, protocol, non_block, close_exec, &err);
  if (!fd) {
    spdlog::error("create_socket error?!");
  }
  decltype(std::make_unique<Socket>(*fd)) sock_ptr;
  try {
    sock_ptr = std::make_unique<Socket>(*fd);
  } catch (const std::exception &e) {
    spdlog::error("create_socket error throw exception: {}", e.what());
  }
  return sock_ptr;
}

} // namespace socket
} // namespace adl
#endif // __SOCKET_H__