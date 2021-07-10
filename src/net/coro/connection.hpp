#ifndef __CONNECTION_H__
#define __CONNECTION_H__
#include "../socket/socket.hpp"
#include "co_epoll.cpp"
#include "co_future_attr.hpp"
#include "co_io_recv.hpp"
#include "co_io_send.hpp"
#include <atomic>
namespace adl {
using io_buffer_reserved_t = std::array<std::byte, 3900>;

using socket::ConnectSocket;

class Connection {
public:
  explicit Connection(int fd);
  void create_reader_corotine();
  void create_writer_corotine();
  void set_poller(const std::shared_ptr<co_epoll> &poller);

  void async_recv();
  void async_send();
  void close() { closed_ = true; }

private:
  std::shared_ptr<co_epoll> poller_;              /* epoll */
  std::unique_ptr<ConnectSocket> connect_fd_ptr_; /* socket */

  struct recv_awaiter {};
  struct send_awaiter {};

  null_future co_reader();
  null_future co_writer();
  recv_awaiter async_recv(void *buf, size_t len);
  send_awaiter async_send(void *buf, size_t len);
  // read_co_handle_; 读协程句柄
  // write_co_handle_; 写协程句柄
  // io_work;

  std::atomic<bool> closed_;

public:
  int error_;
  uint32_t events_;  /* 注册事件 */
  uint32_t revents_; /* 活跃事件 */
};

} // namespace adl
#endif // __CONNECTION_H__