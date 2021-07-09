#ifndef __CONNECTION_H__
#define __CONNECTION_H__
#include "../socket/socket.hpp"
#include "co_epoll.cpp"
#include "co_io_recv.hpp"
#include "co_io_send.hpp"

namespace adl {
using socket::ConnectSocket;

class Connection {
public:
  explicit Connection(int fd);
  void service();
  void link_with_poller(const std::shared_ptr<co_epoll> &poller);

private:
  std::shared_ptr<co_epoll> poller_;              /* epoll */
  std::unique_ptr<ConnectSocket> connect_fd_ptr_; /* socket */
  // read_co_handle_; 读协程句柄
  // write_co_handle_; 写协程句柄
  // io_work;

public:
  uint32_t events_;  /* 注册事件 */
  uint32_t revents_; /* 活跃事件 */
};

} // namespace adl
#endif // __CONNECTION_H__