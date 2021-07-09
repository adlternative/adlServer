#include "co_epoll.hpp"
#include "co_future_attr.hpp"
#include "co_io_recv.hpp"
#include "co_io_send.hpp"
#include <cerrno>

using io_buffer_reserved_t = std::array<std::byte, 3900>;

null_future tcp_echo_service(co_epoll &co_poller, int fd) {
  io_work work(co_poller);
  int rsz = 0, ssz = 0;
  io_buffer_t buf{};
  io_buffer_reserved_t storage{};
RecvData:
  buf = storage;
  rsz = co_await work.async_recv(fd, buf, 0);
  if (rsz == 0)
    co_return;
  /* 错误处理 */

  buf = {storage.data(), static_cast<size_t>(rsz)};

SendData:
  ssz = co_await work.async_send(fd, buf, 0);
  if (ssz == -1 && work.errno_ == EPIPE)
    co_return;
  /* 错误处理 */

  if (ssz == rsz)
    goto RecvData;
  rsz -= ssz;
  buf = {storage.data() + ssz, static_cast<size_t>(rsz)};
  goto SendData;
  co_return;
}

#include <fmt/core.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>

int socket_create(const addrinfo &hint) noexcept {
  return ::socket(hint.ai_family, //
                  hint.ai_socktype, hint.ai_protocol);
}

int socket_bind(int fd, const sockaddr_in &local) noexcept {
  if (::bind(fd, reinterpret_cast<const sockaddr *>(&local),
             sizeof(sockaddr_in))) {
    return errno;
  }
  return 0;
}

int socket_listen(int fd, int n = SOMAXCONN) noexcept {
  ::listen(fd, n); /* 4096 */
  return errno;
}

int socket_accept(int ln, int &fd) noexcept {
  fd = ::accept4(ln, nullptr, nullptr, SOCK_NONBLOCK | SOCK_CLOEXEC);
  // fd = ::accept(ln, nullptr, nullptr);
  return errno;
}
/* g++ co_echo_service.cpp co_io_recv.cpp co_io_send.cpp co_epoll.cpp
co_io_base.cpp -std=c++2a -lfmt */
int main(int argc, char const *argv[]) {
  addrinfo hint{};
  hint.ai_family = AF_INET;
  hint.ai_socktype = SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC;
  hint.ai_protocol = IPPROTO_TCP;

  auto ln = socket_create(hint);
  // fmt::print("fd = {}\n", ln);

  sockaddr_in local{};
  local.sin_family = hint.ai_family;
  local.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  local.sin_port = htons(12345);
  socket_bind(ln, local);
  socket_listen(ln);
  int sd;
  while (socket_accept(ln, sd) && sd == -1)
    ;
  fmt::print("fd = {}\n", sd);
  co_epoll io_service(-1);
  tcp_echo_service(io_service, sd);
  io_service();
  while (1)
    ;
}