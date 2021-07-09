#include "co_reader.hpp"
#include "co_future_attr.hpp"
#include "co_io_recv.hpp"
#include "connections.hpp"
// #include "co_io_send.hpp"

using io_buffer_reserved_t = std::array<std::byte, 3900>;

null_future co_read(std::shared_ptr<adl::Connection> &connect) {
  // io_work work(co_poller);
  int rsz = 0;
  io_buffer_t buf{};
  io_buffer_reserved_t storage{};
  do {
    // buf = storage;
    // rsz = co_await work.async_recv(fd, buf, 0);
    // if (rsz == 0)
    //    break;
    // else if (rsz == -1) {
    //  if (errno == EINTR) {
    // no this errno happen because we have handle it
    //  in await_assume()!
    // } else if (errno == EAGAIN) {continue;}
    // else { log and close the connection;}
    // }
    // [result, respond] = parsing(buf);
    // if (result == ok)
    //    put the respond to respond_queue
    // else if (result == fail)
    //    close the connection
    // else
    //    continue; // to wait for epoll next time wake IN events
    //
  } while (1); // while(not_close)
  co_return;
}