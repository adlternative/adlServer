#include "io_buffer.hpp"
#include <gtest/gtest.h>
#ifndef SPDLOG_FMT_EXTERNAL
#define SPDLOG_FMT_EXTERNAL
#endif
#include <spdlog/spdlog.h>

TEST(IO_BUFFER, io_buffer) {
  spdlog::set_level(spdlog::level::debug);
  auto &&buf = std::make_unique<io_buffer>();
  char ar[20] = "abcd";
  std::span<char> span1{ar, 4};
  buf->append(span1);
  auto span2 = buf->just_read(4);
  EXPECT_EQ(span2.size(), 4);
  EXPECT_EQ(span2.data()[0], 'a');
  EXPECT_EQ(span2.data()[1], 'b');
  EXPECT_EQ(span2.data()[2], 'c');
  EXPECT_EQ(span2.data()[3], 'd');
}