#ifndef SOCKET_H
#define SOCKET_H
#include <arpa/inet.h>
#include <boost/noncopyable.hpp>
// struct tcp_info;

namespace adl {

namespace sock {

int createNonblockingOrDie(sa_family_t family);

int connect(int sockfd, const struct sockaddr *addr);
void bindOrDie(int sockfd, const struct sockaddr *addr);
void listenOrDie(int sockfd);
int accept(int sockfd, struct sockaddr_in *addr);

ssize_t read(int sockfd, void *buf, size_t count);
ssize_t readv(int sockfd, const struct iovec *iov, int iovcnt);
ssize_t write(int sockfd, const void *buf, size_t count);

void close(int sockfd);
void shutdownWrite(int sockfd);
void shutdownRead(int sockfd);
void toIpPort(char *buf, size_t size, const struct sockaddr *addr);
void toIp(char *buf, size_t size, const struct sockaddr *addr);
void toPortString(char *buf, size_t size, const struct sockaddr *addr);
void fromIpPort(const char *ip, uint16_t port, struct sockaddr_in *addr);

int getSocketError(int sockfd);

struct sockaddr *sockaddr_cast(struct sockaddr_in *addr);
const struct sockaddr *sockaddr_cast(const struct sockaddr_in *addr);
// const struct sockaddr *sockaddr_cast(const struct sockaddr_in6 *addr);
// struct sockaddr *sockaddr_cast(struct sockaddr_in6 *addr);
const struct sockaddr_in *sockaddr_in_cast(const struct sockaddr *addr);

// const struct sockaddr_in6 *sockaddr_in6_cast(const struct sockaddr *addr);

struct sockaddr_in getLocalAddr(int sockfd);
struct sockaddr_in getPeerAddr(int sockfd);
bool isSelfConnect(int sockfd);

} // namespace sock
class InetAddress;

class Socket : boost::noncopyable {
public:
  explicit Socket(int sockfd) : sockfd_(sockfd) {}
  ~Socket() { sock::close(sockfd_); }
  int fd() const { return sockfd_; }
  // bool getTcpINFO_(struct tcp_info *) const;
  // bool getTcpInfoString(char *buf, int len) const;
  void bindAddress(const InetAddress &localaddr);
  void listen();
  int accept(InetAddress *peeraddr);
  void shutdownWrite();
  void shutdownRead();
  void setTcpNoDelay(bool on);
  void setReuseAddr(bool on);
  void setReusePort(bool on);
  void setKeepAlive(bool on);

private:
  const int sockfd_;
};
} // namespace adl
#endif
