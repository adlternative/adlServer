#ifndef __SOCKET_H__
#define __SOCKET_H__

#include "../../base/noncopyable.hpp"
#include "socket_wrap.hpp"
#include <memory>
#include <stdexcept>

#ifndef SPDLOG_FMT_EXTERNAL
#define SPDLOG_FMT_EXTERNAL
#endif
#include <spdlog/spdlog.h>

namespace adl {

namespace socket {

class Socket : noncopyable {
public:
  Socket() : socket_fd_(-1) {}
  explicit Socket(int socket_fd) noexcept : socket_fd_(socket_fd) {}
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

  ~Socket() noexcept { this->close(); }

  template <enum socket_family family = ipv4__,
            enum socket_protocol protocol = tcp__>
  static std::unique_ptr<Socket> socket_create(int non_block, int close_exec) {
    int err;

    auto fd = socket_init(family, protocol, non_block, close_exec, &err);
    if (!fd) {
      spdlog::error("socket_create error?!");
    }
    decltype(std::make_unique<Socket>(*fd)) sock_ptr{};
    try {
      sock_ptr = std::make_unique<Socket>(*fd);
    } catch (const std::exception &e) {
      spdlog::error("socket_create error throw exception: {}", e.what());
    }
    return sock_ptr;
  }

  int get_fd() noexcept { return socket_fd_; }

protected:
  int socket_fd_;
};

class ListenSocket : public Socket {
public:
  ListenSocket() : Socket() {}
  explicit ListenSocket(int socket_fd) noexcept : Socket(socket_fd) {}
  template <enum socket_family family = ipv4__>
  static std::unique_ptr<ListenSocket> listen_socket_create(int non_block,
                                                            int close_exec) {
    int err;

    auto fd = socket_init(family, tcp__, non_block, close_exec, &err);
    if (!fd) {
      spdlog::error("socket_create error?!");
    }
    decltype(std::make_unique<ListenSocket>(*fd)) sock_ptr{};
    try {
      sock_ptr = std::make_unique<ListenSocket>(*fd);
    } catch (const std::exception &e) {
      spdlog::error("socket_create error throw exception: {}", e.what());
    }
    return sock_ptr;
  }

  ListenSocket &reuse_addr(int on = true) {
    int err;
    if (socket_set_reuse_addr(socket_fd_, true, &err)) {
      spdlog::error("socket_set_reuse_addr err: {}", strerror(err));
    }
    return *this;
  }
  ListenSocket &reuse_port(int on = true) {
    int err = 0;
    if (socket_set_reuse_port(socket_fd_, true, &err)) {
      spdlog::critical("socket_set_reuse_port err: {}", strerror(err));
    }
    return *this;
  }
  ListenSocket &bind(const char *ip, short port, enum socket_family family) {
    int err = 0;
    if (socket_bind(socket_fd_, ip, port, family, &err)) {
      spdlog::critical("socket_bind err: {}", strerror(err));
    }
    return *this;
  }
  ListenSocket &bind(const struct sockaddr *addr, enum socket_family family) {
    int err = 0;
    if (socket_bind(socket_fd_, addr, family, &err)) {
      spdlog::critical("socket_bind err: {}", strerror(err));
    }
    return *this;
  }
  ListenSocket &listen() {
    int err = 0;
    if (socket_listen(socket_fd_, SOMAXCONN, &err)) {
      spdlog::critical("socket_listen err: {}", strerror(err));
    }
    return *this;
  }
};

class ConnectSocket : public Socket {
public:
  ConnectSocket() noexcept : Socket() {}
  explicit ConnectSocket(int socket_fd) noexcept : Socket(socket_fd) {}
  template <enum socket_family family = ipv4__,
            enum socket_protocol protocol = tcp__>
  static std::unique_ptr<ConnectSocket> connect_socket_create(int fd) {
    decltype(std::make_unique<ConnectSocket>(fd)) sock_ptr{};
    try {
      sock_ptr = std::make_unique<ConnectSocket>(fd);
    } catch (const std::exception &e) {
      spdlog::error("socket_create error throw exception: {}", e.what());
    }
    return sock_ptr;
  }

  int read(int sockfd, void *buf, size_t len) {
    int err = 0;
    int ret = 0;
    for (;;) {
      ret = socket_read(socket_fd_, buf, len, &err);
      if (ret < 0) {
        if (err == EINTR) {
          continue;
        }
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
          spdlog::error("socket_read err: {}", strerror(err));
        }
      }
      break;
    }
    return ret;
  }

  int write(int sockfd, void *buf, size_t len) {
    int err = 0;
    int ret = 0;
    for (;;) {
      ret = socket_write(socket_fd_, buf, len, &err);
      if (ret < 0) {
        if (err == EINTR) {
          continue;
        }
        if (errno == EPIPE) {
          spdlog::info("peer close!");
        }
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
          spdlog::error("socket_write err: {}", strerror(err));
        }
      }
      break;
    }
    return ret;
  }
};

} // namespace socket
} // namespace adl
#endif // __SOCKET_H__