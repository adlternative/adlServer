#include "InetAddress.h"
#include "socket.h"
#include <cstring>
namespace adl {
InetAddress::InetAddress(const char *ip, uint16_t port) {
  bzero(&addr_, sizeof(addr_));
  sockets::fromIpPort(ip, port, &addr_);
}

std::string InetAddress::toIp() const {
  char buf[64] = "";
  sockets::toIp(buf, sizeof buf, getSockAddr());
  return buf;
}

std::string InetAddress::toIpPort() const {
  char buf[64] = "";
  sockets::toIpPort(buf, sizeof buf, getSockAddr());
  return buf;
}

uint16_t InetAddress::toPort() const { return ntohs(addr_.sin_port); }

} // namespace adl