#include "Socket.h"
#include "../util.h"
#include "InetAddress.h"
#include <cassert>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h> // snprintf
#include <sys/socket.h>
#include <sys/uio.h> // readv
#include <unistd.h>

namespace adl {
void Socket::bindAddress(const InetAddress &addr) {
  sock::bindOrDie(sockfd_, addr.getSockAddr());
}

void Socket::listen() { sock::listenOrDie(sockfd_); }

int Socket::accept(InetAddress *peeraddr) {
  struct sockaddr_in addr;
  explicit_bzero(&addr, sizeof addr);
  int connfd = sock::accept(sockfd_, &addr);
  if (connfd >= 0) {
    peeraddr->setSockAddr(addr);
  }
  return connfd;
}

void Socket::shutdownWrite() { sock::shutdownWrite(sockfd_); }

/* 禁用Nagle's algorithm:
等待直到等到前一个发送数据的ACK返回再发送小数据，对Telnet 可能有帮助。
但是等待ACK增加了延时。
https://stackoverflow.com/questions/3761276/when-should-i-use-tcp-nodelay-and-when-tcp-cork
*/
void Socket::setTcpNoDelay(bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &optval,
               static_cast<socklen_t>(sizeof optval));
  // FIXME CHECK
}

/* SO_REUSEADDR允许套接字
  正在使用的端口被另一个SOCKET强制绑定
  https://lwn.net/Articles/542629/ */
void Socket::setReuseAddr(bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval,
               static_cast<socklen_t>(sizeof optval));
  // FIXME CHECK
}

/*  如果多个服务器（进程或线程）分别将选项设置
  SO_REUSEPORT,则它们可以绑定到同一端口：
 SO_REUSEPORT的一个论据是，
它使得在同一套接字上使用独立启动的进程更加容易。
例如，您可以简单地启动新服务器（可能是新版本），
然后关闭旧服务器，而不会中断任何服务。
*/
void Socket::setReusePort(bool on) {
  int optval = on ? 1 : 0;
  int ret = ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &optval,
                         static_cast<socklen_t>(sizeof optval));
  if (ret < 0 && on) {
    // LOG_SYSERR << "SO_REUSEPORT failed.";
  }
}

/* 发送心跳包 */
/* 使用SO_KEEPALIVE套接字选项调用
的getsockopt函数允许应用程序检索
keepalive选项的当前状态。
*/
void Socket::setKeepAlive(bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &optval,
               static_cast<socklen_t>(sizeof optval));
  // FIXME CHECK
}

namespace sock {

int createNonblockingOrDie(sa_family_t family) {
  /* 无阻塞，创建的子进程不继承该socket */
  int sockfd =
      ::socket(family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
  if (sockfd < 0) {
    // LOG(FATAL) << "sock::createNonblockingOrDie";
    ERROR_WITH_ERRNO_STR("sock::createNonblockingOrDie")
  }
  return sockfd;
}

void setNonBlockAndCloseOnExec(int sockfd) {
  // non-block
  int flags = ::fcntl(sockfd, F_GETFL, 0);
  flags |= O_NONBLOCK;
  int ret = ::fcntl(sockfd, F_SETFL, flags);
  if (ret < 0)
    ERROR_WITH_ERRNO_STR("setNonBlockAndCloseOnExec error:");
  // close-on-exec
  flags = ::fcntl(sockfd, F_GETFD, 0);
  flags |= FD_CLOEXEC;
  ret = ::fcntl(sockfd, F_SETFD, flags);
  if (ret < 0)
    ERROR_WITH_ERRNO_STR("setNonBlockAndCloseOnExec error:");
  return;
}

/* 连接 */
int connect(int sockfd, const struct sockaddr *addr) {
  return ::connect(sockfd, addr,
                   static_cast<socklen_t>(sizeof(struct sockaddr_in)));
}

void bindOrDie(int sockfd, const struct sockaddr *addr) {
  int ret =
      ::bind(sockfd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in)));
  if (ret < 0) {
    DIE_WITH_ERRNO_STR("sock::bindOrDie");
  }
}

void listenOrDie(int sockfd) {
  int ret = ::listen(sockfd, SOMAXCONN);
  if (ret < 0) {
    DIE_WITH_ERRNO_STR("sock::listenOrDie");
  }
}

int accept(int sockfd, struct sockaddr_in *addr) {
  socklen_t addrlen = static_cast<socklen_t>(sizeof *addr);
  int connfd = ::accept4(sockfd, sockaddr_cast(addr), &addrlen,
                         SOCK_NONBLOCK | SOCK_CLOEXEC);
  if (connfd < 0) {
    int savedErrno = errno;
    // LOG_SYSERR << "Socket::accept";
    switch (savedErrno) {
    case EAGAIN:
    case ECONNABORTED:
    case EINTR:
    case EPROTO: // ???
    case EPERM:
    case EMFILE: // per-process lmit of open file desctiptor ???
      // expected errors
      errno = savedErrno;
      break;
    case EBADF:
    case EFAULT:
    case EINVAL:
    case ENFILE:
    case ENOBUFS:
    case ENOMEM:
    case ENOTSOCK:
    case EOPNOTSUPP:
      // unexpected errors
      // LOG_FATAL << "unexpected error of ::accept " << savedErrno;
      break;
    default:
      // LOG_FATAL << "unknown error of ::accept " << savedErrno;
      break;
    }
  }
  return connfd;
}

void close(int sockfd) {
  if (::close(sockfd) < 0)
    ERROR_WITH_ERRNO_STR("sock::close");
}

void shutdownWrite(int sockfd) {
  if (::shutdown(sockfd, SHUT_WR) < 0) {
    // LOG_SYSERR << "sock::shutdownWrite";
  }
}

void toIpPort(char *buf, size_t size, const struct sockaddr *addr) {
  toIp(buf, size, addr);
  size_t end = ::strlen(buf);
  const struct sockaddr_in *addr4 = sockaddr_in_cast(addr);
  uint16_t port = ntohs(addr4->sin_port);
  assert(size > end);
  snprintf(buf + end, size - end, ":%u", port);
}
void toPortString(char *buf, size_t size, const struct sockaddr *addr) {
  const struct sockaddr_in *addr4 = sockaddr_in_cast(addr);
  size_t end = ::strlen(buf);
  uint16_t port = ntohs(addr4->sin_port);
  assert(size > end);
  snprintf(buf + end, size - end, "%u", port);
}
void toIp(char *buf, size_t size, const struct sockaddr *addr) {
  if (addr->sa_family == AF_INET) {
    assert(size >= INET_ADDRSTRLEN);
    const struct sockaddr_in *addr4 = sockaddr_in_cast(addr);
    ::inet_ntop(AF_INET, &addr4->sin_addr, buf, static_cast<socklen_t>(size));
  }
}
void fromIpPort(const char *ip, uint16_t port, struct sockaddr_in *addr) {
  addr->sin_family = AF_INET;
  addr->sin_port = htons(port);
  if (::inet_pton(AF_INET, ip, &addr->sin_addr) <= 0) {
    /* LOG */
  }
}

/* 通信 */
ssize_t read(int sockfd, void *buf, size_t count) {
  return ::read(sockfd, buf, count);
}

ssize_t write(int sockfd, const void *buf, size_t count) {
  return ::write(sockfd, buf, count);
}

ssize_t readv(int sockfd, const struct iovec *iov, int iovcnt) {
  return ::readv(sockfd, iov, iovcnt);
}

int getSocketError(int sockfd) {
  int optval;
  socklen_t optlen = static_cast<socklen_t>(sizeof optval);

  if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
    return errno;
  } else {
    return optval;
  }
}

struct sockaddr *sockaddr_cast(struct sockaddr_in *addr) {
  return static_cast<struct sockaddr *>(implicit_cast<void *>(addr));
}

const struct sockaddr *sockaddr_cast(const struct sockaddr_in *addr) {
  return static_cast<const struct sockaddr *>(
      implicit_cast<const void *>(addr));
}

const struct sockaddr_in *sockaddr_in_cast(const struct sockaddr *addr) {
  return static_cast<const struct sockaddr_in *>(
      implicit_cast<const void *>(addr));
}

struct sockaddr_in getLocalAddr(int sockfd) {
  struct sockaddr_in localaddr;
  explicit_bzero(&localaddr, sizeof localaddr);
  socklen_t addrlen = static_cast<socklen_t>(sizeof localaddr);
  if (::getsockname(sockfd, sockaddr_cast(&localaddr), &addrlen) < 0) {
    // LOG_SYSERR << "sock::getLocalAddr";
  }
  return localaddr;
}

struct sockaddr_in getPeerAddr(int sockfd) {
  struct sockaddr_in peeraddr;
  explicit_bzero(&peeraddr, sizeof peeraddr);
  socklen_t addrlen = static_cast<socklen_t>(sizeof peeraddr);
  if (::getpeername(sockfd, sockaddr_cast(&peeraddr), &addrlen) < 0) {
    // LOG_SYSERR << "sock::getPeerAddr";
  }
  return peeraddr;
}

} // namespace sock
} // namespace adl