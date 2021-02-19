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

void close(int sockfd) {
  if (::close(sockfd) < 0)
    ERROR_WITH_ERRNO_STR("sock::close");
}

void shutdownWrite(int sockfd) {
  if (::shutdown(sockfd, SHUT_WR) < 0) {
    // LOG_SYSERR << "sock::shutdownWrite";
  }
}

void toIpPort(char *buf, size_t size, const struct sockaddr *addr) {}

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