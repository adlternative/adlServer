#include "socket.hpp"
#include <fmt/color.h>
#include <fmt/core.h>
#include <gtest/gtest.h>
/*
  g++  socket_test.cpp socket_wrap.cpp socket.cpp  -lgtest -lgtest_main
  -lspdlog -lfmt -std=c++20 && ./a.out
*/
TEST(Socket, create_socket) {
  using adl::socket::Socket;
  auto socket_ptr = Socket::create_socket<ipv4__, tcp__>(1, 1);
}