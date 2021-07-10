#include "socket_wrap.hpp"
#include <sys/socket.h>

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
 * @param close_exec 设置是否在exec时候关闭
 * @return int sockfd
 */
[[nodiscard]] std::optional<int> socket_init(enum socket_family family,
                                             enum socket_protocol protocol,
                                             int non_block, int close_exec,
                                             int *errno_) noexcept {
  int fd =
      ::socket(family == 4 ? AF_INET : AF_INET6,
               ((protocol == tcp__) ? SOCK_STREAM : SOCK_DGRAM) |
                   (non_block * SOCK_NONBLOCK) | (close_exec * SOCK_CLOEXEC),
               protocol == tcp__ ? IPPROTO_TCP : IPPROTO_UDP);
  if (fd < 0 && errno_) {
    *errno_ = errno;
    return {};
  }
  return fd;
}

[[nodiscard]] std::optional<int> socket_init(int &fd, const addrinfo &hint,
                                             int non_block, int close_exec,
                                             int *errno_) noexcept {
  fd = ::socket(hint.ai_family,
                hint.ai_socktype | (non_block * SOCK_NONBLOCK) |
                    (close_exec * SOCK_CLOEXEC),
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
int socket_set_reuse_addr(int sockfd, bool on, int *errno_) noexcept {
  int optval = on ? 1 : 0;
  int ret = ::setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval,
                         static_cast<socklen_t>(sizeof optval));
  if (ret < 0 && errno_) {
    *errno_ = errno;
  }
  return ret;
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
int socket_set_reuse_port(int sockfd, bool on, int *errno_) noexcept {
  int optval = on ? 1 : 0;
  int ret = ::setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &optval,
                         static_cast<socklen_t>(sizeof optval));
  if (ret < 0 && errno_) {
    *errno_ = errno;
  }
  return ret;
}

int socket_bind(int sockfd, const struct sockaddr *addr,
                enum socket_family family, int *errno_) noexcept {
  int ret = ::bind(sockfd, addr,
                   family == ipv4__
                       ? static_cast<socklen_t>(sizeof(struct sockaddr_in))
                       : static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
  if (ret < 0 && errno_) {
    *errno_ = errno;
  }
  return ret;
}

int socket_bind(int sockfd, const char *ip, short port,
                enum socket_family family, int *errno_) noexcept {
  int ret;
  union {
    sockaddr_in addr;
    sockaddr_in6 addr6;
  } socket_in_addr;
  if (family == ipv4__) {
    socket_in_addr.addr.sin_family = AF_INET;
    socket_in_addr.addr.sin_port = htons(port);
    if ((ret = ::inet_pton(socket_in_addr.addr.sin_family, ip,
                           &socket_in_addr.addr.sin_addr)) <= 0 &&
        errno_) {
      *errno_ = errno;
      return ret;
    }
  } else if (family == ipv6__) {
    socket_in_addr.addr6.sin6_family = AF_INET6;
    socket_in_addr.addr6.sin6_port = htons(port);
    if ((ret = ::inet_pton(socket_in_addr.addr6.sin6_family, ip,
                           &socket_in_addr.addr6.sin6_addr)) <= 0 &&
        errno_) {
      *errno_ = errno;
      return ret;
    }
  }

  ret = socket_bind(sockfd, reinterpret_cast<sockaddr *>(&socket_in_addr),
                    family, errno_);
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
 * listen 的第二个参数的含义：告诉了内核全连接队列[Accept队列]大小。“
 * 应用
 * 对于一个高性能服务器来说 backlog “越大越好”，
 * 但是可能会占用很大内存，而且似乎有一些网络攻击的方式
 *  原理
 * 1）server端接受到SYN请求，创建socket，存储于SYN
 * Queue（半连接队列），并向客户端返回SYN+ACK；
 * 2）server端接收到第三次握手的ACK，socket状态更新为ESTABLISHED，同时将socket移动到Accept
 * Queue（全连接队列），等待应用程序执行accept()。
 * 为什么要分为 SYN QUEUE | ACCEPT QUEUE ?
 * 用户只用关心 ACCEPT
 * 队列中的连接长度，而系统可以通过修改半连接队列长度而做到流量控制。
 * 通过系统设置半连接队列[SYN队列]大小(tcp.stat=SYN_RECEIEVE)，
 * 通过应用参数设置全连接大小。”
 * http://veithen.io/2014/01/01/how-tcp-backlog-works-in-linux.html
 * https://www.cnblogs.com/pengyusong/p/6434788.html
 */
int socket_listen(int fd, int n, int *errno_) noexcept {
  int ret = ::listen(fd, n); /* 4096 */
  if (ret < 0 && errno_) {
    *errno_ = errno;
  }
  return ret;
}

/* ---------------accept------------------ */

std::optional<int> socket_accept(int listen_fd, struct sockaddr *addr,
                                 int non_block, int close_exec,
                                 int *errno_) noexcept {
  socklen_t addrlen = static_cast<socklen_t>(sizeof *addr);
  int connfd =
      ::accept4(listen_fd, addr, &addrlen,
                (SOCK_NONBLOCK * non_block) | (close_exec * SOCK_CLOEXEC));
  if (connfd < 0 && errno_) {
    *errno_ = errno;
    return {};
  }
  return connfd;
}

std::optional<int> socket_accept(int listen_fd, struct sockaddr_in *addr,
                                 int non_block, int close_exec,
                                 int *errno_) noexcept {
  return socket_accept(listen_fd, reinterpret_cast<struct sockaddr *>(addr),
                       non_block, close_exec, errno_);
}

std::optional<int> socket_accept(int listen_fd, struct sockaddr_in6 *addr,
                                 int non_block, int close_exec,
                                 int *errno_) noexcept {
  return socket_accept(listen_fd, reinterpret_cast<struct sockaddr *>(addr),
                       non_block, close_exec, errno_);
}

/* ---------------close shutdown------------------ */

int socket_close(int sockfd, int *errno_) noexcept {
  int ret;
  ret = ::close(sockfd);
  if (ret < 0 && errno_) {
    *errno_ = errno;
  }
  return ret;
}

int socket_shutdown_read(int sockfd, int *errno_) noexcept {
  int ret;
  ret = ::shutdown(sockfd, SHUT_RD);
  if (ret < 0 && errno_) {
    *errno_ = errno;
  }
  return ret;
}

int socket_shutdown_write(int sockfd, int *errno_) noexcept {
  int ret;
  ret = ::shutdown(sockfd, SHUT_WR);
  if (ret < 0 && errno_) {
    *errno_ = errno;
  }
  return ret;
}

int socket_shutdown_both(int sockfd, int *errno_) noexcept {
  int ret;
  ret = ::shutdown(sockfd, SHUT_RDWR);
  if (ret < 0 && errno_) {
    *errno_ = errno;
  }
  return ret;
}

int socket_read(int sockfd, void *buf, size_t len, int *errno_) {
  int ret;
  ret = ::read(sockfd, buf, len);
  if (ret < 0 && errno_) {
    *errno_ = errno;
  }
  return ret;
}

int socket_write(int sockfd, void *buf, size_t len, int *errno_) {
  int ret;
  ret = ::write(sockfd, buf, len);
  if (ret < 0 && errno_) {
    *errno_ = errno;
  }
  return ret;
}

} // namespace socket

} // namespace adl