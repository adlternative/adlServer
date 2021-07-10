
#include "io_buffer.hpp"
#ifndef SPDLOG_FMT_EXTERNAL
#define SPDLOG_FMT_EXTERNAL
#endif
#include <spdlog/spdlog.h>

/* shrink_to_fit */

io_buffer::io_buffer(size_t init_size) : vec_(init_size), read_idx_(0) {}

void io_buffer::append(std::span<char> buffer) {
  // spdlog::debug("vec size = {}", vec_.size());
  // spdlog::debug("buffer size = {}", buffer.size());
  // spdlog::debug("available size = {}", available());
  auto &&cur_ = cur_write();

  if (buffer.size() > writeable()) {
    std::copy(buffer.begin(), buffer.begin() + writeable(), cur_);
    for (auto i = buffer.begin() + writeable(); i != buffer.end(); i++) {
      vec_.push_back(*i);
    }
  } else {
    std::copy(buffer.begin(), buffer.end(), cur_);
  }
  write_idx_ += buffer.size();
}

size_t io_buffer::readable() { return write_idx_ - read_idx_; }
size_t io_buffer::writeable() { return vec_.size() - write_idx_; }
size_t io_buffer::available() { return vec_.capacity() - vec_.size(); }

std::span<char> io_buffer::fetch(size_t len) {
  if (len > readable()) {
    spdlog::debug("fetch len = {}, readble = {}", len, readable());
    return std::span<char>{};
  }
  size_t read_idx = read_idx_;
  read_idx_ += len;
  return std::span<char>{vec_.data() + read_idx, len};
}

std::span<char> io_buffer::just_read(size_t len) {
  if (len > readable()) {
    spdlog::debug("fetch len = {}, readble = {}", len, readable());
    return std::span<char>{};
  }
  return std::span<char>{vec_.data() + read_idx_, len};
}