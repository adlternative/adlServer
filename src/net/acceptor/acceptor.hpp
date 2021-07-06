#ifndef __ACCEPTOR_H__
#define __ACCEPTOR_H__

#include "../../base/noncopyable.hpp"
#include "../socket/socket.hpp"
#include <cerrno>
#include <sys/epoll.h>

namespace adl {
namespace acceptor {
using adl::socket::ListenSocket;
using adl::socket::Socket;

class Acceptor : noncopyable {
public:
  Acceptor(const char *ip, short port)
      : stopped_(false),
        listen_fd_ptr_(
            ListenSocket::listen_socket_create<ipv4__, tcp__>(1, 1)) {
    if (!listen_fd_ptr_) {
      spdlog::error("Acceptor() error?!");
    }
    listen_fd_ptr_->reuse_addr().reuse_port().bind(ip, port, ipv4__).listen();
  }
  ~Acceptor() {}
  void stop() { stopped_ = true; }
  int non_block_accept(int non_block = true, int close_exec = true) {
    int err = 0;
    for (; !stopped_;) {
      struct sockaddr addr;
      if (auto connect_fd = adl::socket::socket_accept(
              listen_fd_ptr_->get_fd(), &addr, non_block, close_exec, &err)) {
        spdlog::info("new connection!");
        // create connection(fd, addr)
        // return *connect_fd;
      } else if (err == EAGAIN || err == EWOULDBLOCK) {
        break;
      } else if (err == EINTR) {
        continue;
      } else {
        spdlog::error("Acceptor::to_accept() error: {}", strerror(err));
        return -1;
      }
    }
    if (stopped_) {
      spdlog::info("acceptor going to die!");
      return -1;
    }
    return 0;
  }
  void temp_poller() {
    int temp_epoll = epoll_create1(EPOLL_CLOEXEC);
    epoll_event ee{
        .events = EPOLLET | EPOLLIN,
        .data = {.fd = listen_fd_ptr_->get_fd()},
    };
    epoll_ctl(temp_epoll, EPOLL_CTL_ADD, ee.data.fd, &ee);
    while (!stopped_) {
      int cnt = epoll_wait(temp_epoll, &ee, 1, -1);
      for (int i = 0; i < cnt; i++) {
        non_block_accept();
      }
    }
  }

private:
  bool stopped_;
  std::unique_ptr<ListenSocket> listen_fd_ptr_;
  /* epoll */
};

} // namespace acceptor
} // namespace adl
#endif // __ACCEPTOR_H__