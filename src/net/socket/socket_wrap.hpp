#ifndef __SOCKET_WRAP_H__
#define __SOCKET_WRAP_H__

#include <arpa/inet.h>
#include <cerrno>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <optional>
#include <sys/socket.h>
#include <unistd.h>

enum socket_family { ipv4__ = 4, ipv6__ = 6 };
enum socket_protocol { tcp__, udp__ };

/*  AF_UNSPEC */

namespace adl {

namespace socket {

int __attribute__((noinline)) socket_recent() noexcept;
int get_socket_err(int sockfd) noexcept;
[[nodiscard]] std::optional<int> socket_init(enum socket_family family,
                                             enum socket_protocol protocol,
                                             int non_block = true,
                                             int close_exec = true,
                                             int *errno_ = nullptr) noexcept;

[[nodiscard]] std::optional<int> socket_init(int &fd, const addrinfo &hint,
                                             int non_block = true,
                                             int close_exec = true,
                                             int *errno_ = nullptr) noexcept;

int socket_set_reuse_addr(int sockfd, bool on = true,
                          int *errno_ = nullptr) noexcept;

int socket_set_reuse_port(int sockfd, bool on, int *errno_ = nullptr) noexcept;

int socket_bind(int sockfd, const struct sockaddr *addr,
                enum socket_family family, int *errno_ = nullptr) noexcept;

int socket_bind(int sockfd, const char *ip, short port,
                enum socket_family family, int *errno_ = nullptr) noexcept;

int socket_listen(int fd, int n = SOMAXCONN, int *errno_ = nullptr) noexcept;

[[nodiscard]] std::optional<int>
socket_accept(int listen_fd, struct sockaddr *addr, int non_block = true,
              int close_exec = true, int *errno_ = nullptr) noexcept;

[[nodiscard]] std::optional<int>
socket_accept(int listen_fd, struct sockaddr_in *addr, int non_block = true,
              int close_exec = true, int *errno_ = nullptr) noexcept;

[[nodiscard]] std::optional<int>
socket_accept(int listen_fd, struct sockaddr_in6 *addr, int non_block = true,
              int close_exec = true, int *errno_ = nullptr) noexcept;

int socket_close(int sockfd, int *errno_ = nullptr) noexcept;

int socket_shutdown_read(int sockfd, int *errno_ = nullptr) noexcept;

int socket_shutdown_write(int sockfd, int *errno_ = nullptr) noexcept;

int socket_shutdown_both(int sockfd, int *errno_ = nullptr) noexcept;

int socket_read(int sockfd, void *buf, size_t len, int *errno_ = nullptr);
int socket_write(int sockfd, void *buf, size_t len, int *errno_);
} // namespace socket
} // namespace adl

#endif // __SOCKET_WRAP_H__