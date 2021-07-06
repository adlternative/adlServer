#include "socket_wrap.hpp"
#include <fmt/color.h>
#include <fmt/core.h>
#include <gtest/gtest.h>

/* g++ socket_wrap_test.cpp socket_wrap.cpp -lgtest -lgtest_main -fmt*/
TEST(socket, socket_init) {
  int err = 0;
  auto listen_fd = adl::socket::socket_init(ipv4__, tcp__, true, true, &err);
  ASSERT_GT(*listen_fd, 0);
  EXPECT_EQ(adl::socket::socket_close(*listen_fd, &err), 0) << fmt::format(
      fg(fmt::color::crimson), "listen_fd = {}, {}", *listen_fd, strerror(err));

  listen_fd = adl::socket::socket_init(ipv4__, udp__, true, true, &err);
  ASSERT_GT(*listen_fd, 0);
  EXPECT_EQ(adl::socket::socket_close(*listen_fd, &err), 0) << fmt::format(
      fg(fmt::color::crimson), "listen_fd = {}, {}", *listen_fd, strerror(err));
  listen_fd = adl::socket::socket_init(ipv6__, tcp__, true, true, &err);
  ASSERT_GT(*listen_fd, 0);
  EXPECT_EQ(adl::socket::socket_close(*listen_fd, &err), 0) << fmt::format(
      fg(fmt::color::crimson), "listen_fd = {}, {}", *listen_fd, strerror(err));
  listen_fd = adl::socket::socket_init(ipv6__, udp__, true, true, &err);
  ASSERT_GT(*listen_fd, 0);
  EXPECT_EQ(adl::socket::socket_close(*listen_fd, &err), 0) << fmt::format(
      fg(fmt::color::crimson), "listen_fd = {}, {}", *listen_fd, strerror(err));
}

TEST(socket, socket_listen) {
  int err;
  int ret;
  auto listen_fdp = adl::socket::socket_init(ipv4__, tcp__, true, true, &err);
  auto listen_fd = *listen_fdp;
  ASSERT_GT(listen_fd, 0);
  EXPECT_EQ(
      adl::socket::socket_bind(listen_fd, "127.0.0.1", 12345, ipv4__, &err), 0);
  EXPECT_EQ(adl::socket::socket_set_reuse_addr(listen_fd, 1, &err), 0);
  EXPECT_EQ(adl::socket::socket_set_reuse_port(listen_fd, 1, &err), 0);
  EXPECT_EQ(adl::socket::socket_listen(listen_fd, SOMAXCONN, &err), 0);
  EXPECT_EQ(adl::socket::socket_close(listen_fd), 0);
}