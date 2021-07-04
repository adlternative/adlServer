#include "co_io_base.hpp"
#include "co_epoll.hpp"

io_work::io_work(co_epoll &io_service) : io_service_(io_service) {}
