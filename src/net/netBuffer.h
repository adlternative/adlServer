#ifndef NETBUFFER_H
#define NETBUFFER_H
#include "../include/headFile.h"
#include <algorithm>
#include <assert.h>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <memory>
#include <netinet/in.h>
#include <stddef.h>
#include <string>
#include <vector>
namespace adl {
#ifdef AAA
class netBuffer {
public:
  netBuffer() : w_(0), r_(0) {}
  ~netBuffer() {}
  void setW(int len); /* 写完 */
  void setR(int len); /* 取完 */

  /* 向后写 */
  void append(const char *buf, int len) {
    for (int i = 0; i != len; i++) {
      d_.push_back(buf[i]);
    }
    w_ += len;
  }
  void append(char *buf, int len) {
    for (int i = 0; i != len; i++) {
      d_.push_back(buf[i]);
    }
    w_ += len;
  }
  void append(void *buf, int len) { append(static_cast<char *>(buf), len); }
  void append(const void *buf, int len) const {
    append(static_cast<const char *>(buf), len);
  }

  /* 缓冲区的开头 */
  const char *begin() const { return &d_.front(); }
  char *begin() { return &d_.front(); }

  /* 读netBuffer中的内容 */
  const char *peek() const { return begin() + r_; }
  char *peek() { return begin() + r_; };

  void retrieve(size_t len) { r_ += len; }

private:
  std::deque<char> d_;
  int r_;
  int w_; /* 写 */
};

} // namespace adl
#else
using std::string;
class netBuffer {
public:
  netBuffer(int initSize = 4096) : alloc_(initSize), r_(0), w_(0) {
    buf_ = static_cast<char *>(malloc(initSize));
  }
  ~netBuffer() { free(buf_); }
  /* 向后写 */
  void append(const char *buf, int len) {
    LOG(TRACE) << "append" << adl::endl;
    checkCapcity(len);
    for (int i = 0; i != len; i++) {
      buf_[w_++] = buf[i];
    }
  }
  void append(char *buf, int len) {
    LOG(TRACE) << "append" << adl::endl;
    checkCapcity(len);
    for (int i = 0; i != len; i++) {
      buf_[w_++] = buf[i];
    }
  }
  void append(void *buf, int len) { append(static_cast<char *>(buf), len); }
  void append(const void *buf, int len) {
    append(static_cast<const char *>(buf), len);
  }
  /* 读 */
  char *peek() { return buf_ + r_; }
  int peekInt() const {
    int ret;

    assert(readable() >= sizeof(int));
    memcpy(&ret, peek(), sizeof(int));
    return ret;
  }
  int readInt() {
    int i = peekInt();
    retrieve(sizeof(int));
    return i;
  }

  const char *peek() const { return buf_ + r_; }

  const char *begin() const { return buf_; }
  char *begin() { return buf_; }
  /* 读后 */
  void retrieve(int len) {
    // ERR_("%d%d\n", len, readable());
    LOG(TRACE) << "retrieve" << adl::endl;

    assert(len <= readable());
    r_ += len;
  }
  /* 读到string */
  string retrieveAsString(size_t len) {
    LOG(TRACE) << "retrieveAsString" << adl::endl;

    assert(len <= readable());
    string result(peek(), len);
    retrieve(len);
    return result;
  }
  string retrieveAllAsString() {
    LOG(TRACE) << "retrieveAllAsString" << adl::endl;
    return retrieveAsString(readable());
  }
  char *buffer() { return buf_; }
  /* 检查容量,能否往后写;不能则扩容 */
  void checkCapcity(int len) {
    LOG(TRACE) << "checkCapcity" << adl::endl;

    /* 检查缓冲区能否向前移动 */
    if (writeable() <= len) {
      if (checkCanMoveToLeft()) {
        /* move to left */
        memmove(buf_, buf_ + r_, w_ - r_);
        w_ = w_ - r_;
        r_ = 0;
      }
      /* 如果容量仍然不够 */
      if (writeable() <= len)
        /* 扩容 */
        expansion(len);
    }
  }
  /* 扩容 */
  void expansion(int len) {
    LOG(TRACE) << "expansion" << adl::endl;
    while (writeable() <= len)
      alloc_ *= 2;
    buf_ = static_cast<char *>(realloc(buf_, alloc_));
  }
  /* 可读长度 */
  int readable() const { return w_ - r_; }
  /* 可写长度，当“不可写”，buffer扩容 */
  int writeable() const { return alloc_ - w_; }

  int getReadIndex() { return r_; }
  int getWriteIndex() { return w_; }
  int getCapcity() { return alloc_; }
  /* 从一个文件描述符(套接字)读取内容 ，
    无阻塞下 ET 模式 得一次读完*/
  int readFd(int fd, int *savedErrno, bool *closed);
  void reset() {
    r_ = 0;
    w_ = 0;
  }
  void debug() {
    LOG(DEBUG) << "r_:" << r_ << " w_:" << w_ << " buffer:" << buf_
               << adl::endl;
    LOG(DEBUG) << "readable:" << readable() << " writeable:" << writeable()
               << adl::endl;
  }
  void debugLenByte(int len) {
    for (int i = 0; i < len; i++) {
      if (buf_[i] == '\r')
        fprintf(stderr, "(\\r)%d\n", buf_[i]);
      else if (buf_[i] == '\n')
        fprintf(stderr, "(\\n)%d\n", buf_[i]);
      else
        fprintf(stderr, "(%c)%d\n", buf_[i], buf_[i]);
    }
  }

private:
  /* 检查缓冲区能否向前移动 */
  bool checkCanMoveToLeft() { return r_ > 0; }

  char *buf_; /* 缓冲区 */
  int r_;     /* 读坐标 */
  int w_;     /* 写坐标 */
  int alloc_; /* 容量 */
};

#endif
} // namespace adl
#endif
