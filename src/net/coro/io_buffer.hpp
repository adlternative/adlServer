#ifndef __IO_BUFFER_H__
#define __IO_BUFFER_H__

#include <cstddef>
#include <memory>
#include <span>
#include <vector>

class io_buffer {
public:
  explicit io_buffer(size_t init_size = 4096);
  void append(std::span<char> buf);
  size_t readable();
  size_t writeable();
  size_t available();
  char *cur_read() { return vec_.data() + read_idx_; };
  char *cur_write() { return vec_.data() + write_idx_; };
  std::span<char> fetch(size_t len);
  std::span<char> just_read(size_t len);

private:
  size_t read_idx_;
  size_t write_idx_;
  std::vector<char> vec_;
};
#endif // __IO_BUFFER_H__