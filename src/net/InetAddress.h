#ifndef INETADDRESS_H
#define INETADDRESS_H
#include <netinet/in.h>
#include <string>

namespace adl {
namespace sockets {
const struct sockaddr *sockaddr_cast(const struct sockaddr_in *addr);
}

class InetAddress {
public:
  InetAddress(const char *ip, uint16_t port);
  explicit InetAddress(const struct sockaddr_in &addr) : addr_(addr) {}
  std::string toIp() const;
  std::string toIpPort() const;
  uint16_t toPort() const;

  // default copy/assignment are Okay
  void setSockAddr(const struct sockaddr_in &addr) { addr_ = addr; }

  const struct sockaddr *getSockAddr() const {
    return sockets::sockaddr_cast(&addr_);
  }

private:
  struct sockaddr_in addr_;
};

} // namespace adl
#endif
