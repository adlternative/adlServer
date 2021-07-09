#include "co_writer.hpp"
#include "co_future_attr.hpp"
#include "connections.hpp"

// using io_buffer_reserved_t = std::array<std::byte, 3900>;

null_future co_write(std::shared_ptr<adl::Connection> &connect /* int fd */) {
  // io_work work(co_poller);
  int ssz = 0;
  io_buffer_t buf{};
  // buf = connection.respond queue;
  do {
    // ssz = co_await work.async_send(fd, buf, 0);
    // if (ssz == -1) {
    // if( errno_ == EPIPE)
    // continue;
    // else break;
    // }
    /* 写完可以执行一个 write_handle() 打打日志啥的*/
  } while (1);
  co_return;
}
