#include "InetAddress.h"
#include "Socket.h"
#include <cstring>
namespace adl {
InetAddress::InetAddress() {
  explicit_bzero(&addr_, sizeof addr_);
  addr_.sin_family = AF_INET;
  in_addr_t ip = INADDR_ANY;
  addr_.sin_addr.s_addr = htonl(ip);
  addr_.sin_port = htons(0);
}

InetAddress::InetAddress(const char *ip, uint16_t port) {
  explicit_bzero(&addr_, sizeof(addr_));
  sock::fromIpPort(ip, port, &addr_);
}

std::string InetAddress::toIp() const {
  char buf[64] = "";
  sock::toIp(buf, sizeof buf, getSockAddr());
  return buf;
}

std::string InetAddress::toIpPort() const {
  char buf[64] = "";
  sock::toIpPort(buf, sizeof buf, getSockAddr());
  return buf;
}

uint16_t InetAddress::toPort() const { return ntohs(addr_.sin_port); }
std::string InetAddress::toPortString() const {
  char buf[6] = "";
  sock::toPortString(buf, sizeof buf, getSockAddr());
  return buf;
}

} // namespace adl