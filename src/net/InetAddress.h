#ifndef INETADDRESS_H
#define INETADDRESS_H
#include "Socket.h"
#include <netinet/in.h>
#include <string>

namespace adl {

class InetAddress {
public:
  explicit InetAddress();
  explicit InetAddress(const char *ip, uint16_t port);
  explicit InetAddress(const struct sockaddr_in &addr) : addr_(addr) {}
  std::string toIp() const;
  std::string toIpPort() const;
  std::string toPortString() const;

  uint16_t toPort() const;

  sa_family_t family() const { return addr_.sin_family; }

  // default copy/assignment are Okay
  void setSockAddr(const struct sockaddr_in &addr) { addr_ = addr; }

  const struct sockaddr *getSockAddr() const {
    return sock::sockaddr_cast(&addr_);
  }

private:
  struct sockaddr_in addr_;
};

} // namespace adl
#endif
