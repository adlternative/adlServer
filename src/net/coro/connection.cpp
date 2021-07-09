#include "connection.hpp"

namespace adl {
Connection::Connection(int fd)
    : connect_fd_ptr_(ConnectSocket::connect_socket_create(fd)) {}

void Connection::service() {
  // create_reader_corotine();
  // create_writer_corotine();
}

void Connection::link_with_poller(const std::shared_ptr<co_epoll> &poller) {
  poller_ = poller;
}

} // namespace adl