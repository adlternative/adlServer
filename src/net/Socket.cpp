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
// bool Socket::getTcpInfo(struct tcp_info *tcpi) const {
//   socklen_t len = sizeof(struct tcp_info);
//   bzero(tcpi, len);
//   return ::getsockopt(sockfd_, SOL_TCP, TCP_INFO, tcpi, &len) == 0;
// }

// bool Socket::getTcpInfoString(char *buf, int len) const {
//   struct tcp_info tcpi;
//   bool ok = getTcpInfo(&tcpi);
//   if (ok) {
//     snprintf(
//         buf, len,
//         "unrecovered=%u "
//         "rto=%u ato=%u snd_mss=%u rcv_mss=%u "
//         "lost=%u retrans=%u rtt=%u rttvar=%u "
//         "sshthresh=%u cwnd=%u total_retrans=%u",
//         tcpi.tcpi_retransmits, // Number of unrecovered [RTO] timeouts
//         tcpi.tcpi_rto,         // Retransmit timeout in usec
//         tcpi.tcpi_ato,         // Predicted tick of soft clock in usec
//         tcpi.tcpi_snd_mss, tcpi.tcpi_rcv_mss,
//         tcpi.tcpi_lost,    // Lost packets
//         tcpi.tcpi_retrans, // Retransmitted packets out
//         tcpi.tcpi_rtt,     // Smoothed round trip time in usec
//         tcpi.tcpi_rttvar,  // Medium deviation
//         tcpi.tcpi_snd_ssthresh, tcpi.tcpi_snd_cwnd,
//         tcpi.tcpi_total_retrans); // Total retransmits for entire connection
//   }
//   return ok;
// }

void Socket::bindAddress(const InetAddress &addr) {
  sockets::bindOrDie(sockfd_, addr.getSockAddr());
}
void Socket::listen() { sockets::listenOrDie(sockfd_); }
/* 接受一个对端连接 */
int Socket::accept(InetAddress *peeraddr) {
  struct sockaddr_in addr;
  bzero(&addr, sizeof addr);
  int connfd = sockets::accept(sockfd_, &addr);
  if (connfd >= 0) {
    peeraddr->setSockAddr(addr);
  }
  return connfd;
}

void Socket::setTcpNoDelay(bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &optval,
               static_cast<socklen_t>(sizeof optval));
  // FIXME CHECK
}
void Socket::setReuseAddr(bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval,
               static_cast<socklen_t>(sizeof optval));
  // FIXME CHECK
}
void Socket::setReusePort(bool on) {
  int optval = on ? 1 : 0;
  int ret = ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &optval,
                         static_cast<socklen_t>(sizeof optval));
  if (ret < 0 && on) {
    // LOG_SYSERR << "SO_REUSEPORT failed.";
  }
}
void Socket::setKeepAlive(bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &optval,
               static_cast<socklen_t>(sizeof optval));
  // FIXME CHECK
}

/* 写端关闭 */
void Socket::shutdownWrite() { sockets::shutdownWrite(sockfd_); }

namespace sockets {
int createNonblockingOrDie(sa_family_t family) {
  /* 无阻塞，创建的子进程不继承该socket */
  int sockfd =
      ::socket(family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
  if (sockfd < 0) {
    // LOG(FATAL) << "sockets::createNonblockingOrDie";
    ERROR_WITH_ERRNO_STR("sockets::createNonblockingOrDie")
  }
  return sockfd;
}

int connect(int sockfd, const struct sockaddr *addr) {
  return ::connect(sockfd, addr,
                   static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
}

ssize_t read(int sockfd, void *buf, size_t count) {
  return ::read(sockfd, buf, count);
}

void bindOrDie(int sockfd, const struct sockaddr *addr) {
  int ret =
      ::bind(sockfd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
  if (ret < 0) {
    DIE_WITH_ERRNO_STR("sockets::bindOrDie");
  }
}

ssize_t readv(int sockfd, const struct iovec *iov, int iovcnt) {
  return ::readv(sockfd, iov, iovcnt);
}

void listenOrDie(int sockfd) {
  int ret = ::listen(sockfd, SOMAXCONN);
  if (ret < 0) {
    DIE_WITH_ERRNO_STR("sockets::listenOrDie");
  }
}

ssize_t write(int sockfd, const void *buf, size_t count) {
  return ::write(sockfd, buf, count);
}

int accept(int sockfd, struct sockaddr_in6 *addr) {
  socklen_t addrlen = static_cast<socklen_t>(sizeof *addr);

  int connfd = ::accept4(sockfd, sockaddr_cast(addr), &addrlen,
                         SOCK_NONBLOCK | SOCK_CLOEXEC);
  if (connfd < 0) {
    /* TODO */
    ERROR_WITH_ERRNO_STR("sockets::accept");
  }
  return connfd;
}

void close(int sockfd) {
  if (::close(sockfd) < 0)
    ERROR_WITH_ERRNO_STR("sockets::close");
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
  ERROR_WITH_ERRNO_STR("setNonBlockAndCloseOnExec error:");
  return;
}
/*  通过sockaddr 获得ip 和 port */
void toIpPort(char *buf, size_t size, const struct sockaddr *addr) {
  /* ip to buf */
  toIp(buf, size, addr);
  size_t end = ::strlen(buf);
  const struct sockaddr_in *addr4 = sockaddr_in_cast(addr);
  /* port to buf */
  uint16_t port = ntohs(addr4->sin_port);
  assert(size > end);
  snprintf(buf + end, size - end, ":%u", port);
}
/* sockaddr中的IP填入到 buf*/
void toIp(char *buf, size_t size, const struct sockaddr *addr) {
  if (addr->sa_family == AF_INET) {
    assert(size >= INET_ADDRSTRLEN);
    const struct sockaddr_in *addr4 = sockaddr_in_cast(addr);
    ::inet_ntop(AF_INET, &addr4->sin_addr, buf, static_cast<socklen_t>(size));
  }
}

void fromIpPort(const char *ip, uint16_t port, struct sockaddr_in *addr) {
  addr->sin_family = AF_INET;
  port = htons(port);
  if (::inet_pton(AF_INET, ip, &addr->sin_addr) <= 0) {
    /* LOG */
  }
}

// int getSocketError(int sockfd) {}

} // namespace sockets
} // namespace adl