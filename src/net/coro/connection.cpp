#include "connection.hpp"
#ifndef SPDLOG_FMT_EXTERNAL
#define SPDLOG_FMT_EXTERNAL
#endif
#include <spdlog/spdlog.h>

    namespace adl {
  Connection::Connection(int fd)
      : connect_fd_ptr_(ConnectSocket::connect_socket_create(fd)) {}

  void Connection::service() {
    // create_reader_corotine();
    // create_writer_corotine();
  }

  void Connection::set_poller(const std::shared_ptr<co_epoll> &poller) {
    poller_ = poller;
  }

  null_future Connection::co_reader() {
    int rsz = 0;
    do {
      rsz = co_await async_recv();
      if (rsz == 0)
        break;
      else if (rsz < 0) {
        if (error_ == EINTR) {
          // no this errno happen because we have handle it in await_assume() !
          spdlog::debug("async_recv err:", strerror(error_));
        } else if (error_ == EAGAIN || error_ == EWOULDBLOCK) {
          continue;
        } else {
          spdlog::info("peer closed");
          // close;
        }
      } else {
        // [ result, respond ] = parsing(buf);
        // if (result == ok)
        //   put the respond to respond_queue else if (result == fail)
        //       close the connection else continue; // to wait for epoll next
        //       time
        //                                           // wake IN events
      }
    } while (!closed_);
    co_return;
  }

  null_future Connection::co_writer() {
    int wsz = 0;
    co_await async_write();
    co_return;
  }

} // namespace adl