#ifndef ADLBUFFER_H
#define ADLBUFFER_H
#include <cassert>
#include <cstddef>
#include <cstring>
#include <string>
namespace adl {
const int kSmallBuffer = 4000;
const int kLargeBuffer = 4000 * 1000;

template <size_t SIZE> class adlBuffer {

public:
  adlBuffer() : len_(0) {}

  ~adlBuffer() { reset(false); }
  /* 设置len_, max(len_) =SIZE-1 */
  void setlen(size_t len) {
    assert(len < SIZE);
    len_ = len;
    buf_[len_] = '\0';
  }
  void addlen(size_t len) { setlen(len + len_); }
  void append(const char *buf, size_t len) {
    assert(len <= avail());
    memcpy(buf_ + len_, buf, len);
    addlen(len);
  }
  void append(std::string s, size_t len) {
    assert(len <= avail());
    memcpy(buf_ + len_, s.c_str(), len);
    addlen(len);
  }
  void reset(bool ifClear = false) {
    if (ifClear)
      clear();
    setlen(0);
  }
  void clear() { bzero(buf_, SIZE); }
  size_t size() const { return len_; }
  const char *debugString(bool out_to_std = true) {
    if (!strchr(buf_, '\0'))
      buf_[len_] = '\0';
    if (out_to_std) {
      fprintf(stderr, "buf:%s\nsize:%d\n", buf_, len_);
    }
    for (size_t i = 0; i < len_; i++) {
      fprintf(stderr, "%d: %c %d\n", i, buf_[i], buf_[i]);
    }
    return buf_;
  }
  std::string toString() { return std::string(buf_, len_); }
  /* 返回当前buffer最后的字符之后的字符 */
  char *current() { return buf_ + len_; }
  const char *current() const { return buf_ + len_; }
  char *begin() { return buf_; }
  const char *begin() const { return buf_; }
  /* 还剩下多少空间 */
  size_t avail() {
    assert(len_ < SIZE);
    return SIZE - 1 - len_;
  }

private:
  /* 返回缓冲区最后的后面 */
  const char *end() const { return buf_ + SIZE; }

  size_t len_; /* 填入的字符的长度 */
  char buf_[SIZE];
};

} // namespace adl
#endif
