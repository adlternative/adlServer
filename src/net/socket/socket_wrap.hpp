#ifndef __SOCKET_WRAP_H__
#define __SOCKET_WRAP_H__

#if __cplusplus < 202002L
#undef __cplusplus
#define __cplusplus 202002L
#endif

#include <cerrno>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <optional>
#include <sys/socket.h>

enum socket_family { ipv4__ = 4, ipv6__ = 6 };
enum socket_protocol { tcp__, udp__ };

/*  AF_UNSPEC */

namespace adl {

namespace socket {

int __attribute__((noinline)) socket_recent() noexcept;
} // namespace socket
} // namespace adl

#endif // __SOCKET_WRAP_H__