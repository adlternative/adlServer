#include "socket_wrap.hpp"

namespace adl {

namespace socket {

/* ---------------get err------------------ */

int __attribute__((noinline)) socket_recent() noexcept { return errno; }

int get_socket_err(int sockfd) noexcept {
  int optval;
  socklen_t optlen = static_cast<socklen_t>(sizeof optval);

  if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
    return errno;
  } else {
    return optval;
  }
}

/* ---------------socket------------------ */

/**
 * @brief Create a socket object
 *
 * @param family
 * @param protocol http://blog.chinaunix.net/uid-10298232-id-2964533.html
 * 用来指明所要接收的协议包。一般来说，想接收什么样的数据包，就应该在参数protocol里来指定相应的协议。
 * @param non_block 设置是否非阻塞
 * @param close_exec_ 设置是否在exec时候关闭
 * @return int sockfd
 */
[[nodiscard]] std::optional<int> socket_init(enum socket_family family,
                                             enum socket_protocol protocol,
                                             int non_block, int close_exec_,
                                             int *errno_ = nullptr) noexcept {
  int fd = ::socket(family == 4 ? AF_INET : AF_INET6,
                    SOCK_STREAM | (non_block * SOCK_NONBLOCK) |
                        (close_exec_ * SOCK_CLOEXEC),
                    protocol == tcp__ ? IPPROTO_TCP : IPPROTO_UDP);
  if (fd < 0 && errno_) {
    *errno_ = errno;
    return {};
  }
  return fd;
}

[[nodiscard]] std::optional<int> socket_init(int &fd, const addrinfo &hint,
                                             int non_block, int close_exec_,
                                             int *errno_ = nullptr) noexcept {
  fd = ::socket(hint.ai_family,
                hint.ai_socktype | (non_block * SOCK_NONBLOCK) |
                    (close_exec_ * SOCK_CLOEXEC),
                hint.ai_protocol);
  if (fd < 0 && errno_) {
    *errno_ = errno;
    return {};
  }
  return fd;
}

/* ---------------setsockopt + bind------------------ */

/**
 * @brief socket 地址复用
 *
 * @param sockfd
 * @param on
 * @param errno_
 * 功能上：是为了避免了 TIME-WAIT 下 2msl的等待时间
 * 原理上 表示可以重用本地地址、本地端口 (还有一些额外功能)
 * https://blog.csdn.net/whatday/article/details/104598200
 */
void socket_set_reuse_addr(int sockfd, bool on,
                           int *errno_ = nullptr) noexcept {
  int optval = on ? 1 : 0;
  int ret = ::setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval,
                         static_cast<socklen_t>(sizeof optval));
  if (ret < 0 && errno_) {
    *errno_ = errno;
  }
  return;
}

/**
 * @brief socket 端口复用
 *
 * @param sockfd
 * @param on
 * @param errno_
 * 功能上：acceptor
 * 线程可能压力过大，造成单点问题（性能瓶颈），使用端口复用可以将
 * 实现在同一端口上的accept()中阻塞的所有线程（或进程）之间均匀分布连接。
 * 不会出现惊群问题
 * 原理上 端口复用
 * https://lwn.net/Articles/542629/
 */
void socket_set_reuse_port(int sockfd, bool on,
                           int *errno_ = nullptr) noexcept {
  int optval = on ? 1 : 0;
  int ret = ::setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &optval,
                         static_cast<socklen_t>(sizeof optval));
  if (ret < 0 && errno_) {
    *errno_ = errno;
  }
  return;
}

int socket_bind(int sockfd, const struct sockaddr *addr,
                int *errno_ = nullptr) noexcept {
  int ret =
      ::bind(sockfd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in)));
  if (ret < 0 && errno_) {
    *errno_ = errno;
  }
  return ret;
}

/* ---------------listen------------------ */

/**
 * @brief listen socket
 *
 * @param fd
 * @param n
 * @param errno_
 * @return int
 */
int socket_listen(int fd, int n = SOMAXCONN, int *errno_ = nullptr) noexcept {
  int ret = ::listen(fd, n); /* 4096 */
  if (ret < 0 && errno_) {
    *errno_ = errno;
  }
  return ret;
}

} // namespace socket

} // namespace adl