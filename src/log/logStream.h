#ifndef LOGSTREAM_H
#define LOGSTREAM_H
#include "adlBuffer.h"
#include <boost/noncopyable.hpp>
#include <string>
namespace adl {

struct buf_len {
  buf_len(const char *buf, int len) : buf_(buf), len_(len) {}
  buf_len(const void *buf, int len)
      : buf_(static_cast<const char *>(buf)), len_(len) {}

  const char *buf_;
  int len_;
};

class logStream : boost::noncopyable {
public:
  friend logStream &endl(logStream &stream);

  typedef logStream self;
  typedef adlBuffer<kSmallBuffer> Buffer;
  template <typename T> void formatInteger(T v);
  self &operator<<(const buf_len &);
  self &operator<<(self &(*p)(self &stream)) { return p(*this); }
  self &operator<<(short);
  self &operator<<(bool);
  self &operator<<(unsigned short);
  self &operator<<(int);
  self &operator<<(unsigned int);
  self &operator<<(long);
  self &operator<<(unsigned long);
  self &operator<<(long long);
  self &operator<<(unsigned long long);
  self &operator<<(const void *);
  self &operator<<(float);
  self &operator<<(double);
  self &operator<<(char);
  self &operator<<(const char *);
  self &operator<<(const unsigned char *);
  self &operator<<(const std::string &);
  self &operator<<(const Buffer &);
  Buffer &getBuf() { return buf_; }
  void resetBuffer() { buf_.reset(); }
  using outFunc = void (*)(const char *str, size_t len);
  void setGout(outFunc func);

private:
  Buffer buf_;
};
logStream &endl(logStream &stream);

} // namespace adl
#endif
