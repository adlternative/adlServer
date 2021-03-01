#include "logStream.h"
#include <iostream>
namespace adl {

logStream &endl(logStream &stream) {
  if (stream.buf_.avail() >= 1) {
    stream.buf_.append('\n');
  }
  // flush
  return stream;
}

template <typename T> void logStream::formatInteger(T v) {
  auto s = std::to_string(v);
  auto len = s.size();
  if (len <= buf_.avail())
    buf_.append(s, len);
}

logStream &logStream::operator<<(const buf_len &bl) {
  if (bl.len_ <= buf_.avail())
    buf_.append(bl.buf_, bl.len_);
  return *this;
}

logStream &logStream::operator<<(bool v) {
  if (1 <= buf_.avail())
    buf_.append(v ? "1" : "0", 1);
  return *this;
}

logStream &logStream::operator<<(short v) {
  return *this << static_cast<int>(v);
}

logStream &logStream::operator<<(unsigned short v) {
  return *this << static_cast<unsigned int>(v);
}

logStream &logStream::operator<<(int v) {
  formatInteger(v);
  return *this;
}

logStream &logStream::operator<<(unsigned int v) {
  formatInteger(v);
  return *this;
}

logStream &logStream::operator<<(long v) {
  formatInteger(v);
  return *this;
}

logStream &logStream::operator<<(unsigned long v) {
  formatInteger(v);
  return *this;
}

logStream &logStream::operator<<(long long v) {
  formatInteger(v);
  return *this;
}

logStream &logStream::operator<<(unsigned long long v) {
  formatInteger(v);
  return *this;
}

logStream &logStream::operator<<(const void *p) {
  if (32 <= buf_.avail()) {
    int len = snprintf(buf_.current(), 32, "%p", p);
    buf_.addlen(len);
  }
  return *this;
}

logStream &logStream::operator<<(float v) {
  return *this << static_cast<double>(v);
}

logStream &logStream::operator<<(double v) {
  if (32 <= buf_.avail()) {
    int len = snprintf(buf_.current(), 32, "%.12g", v);
    buf_.addlen(len);
  }
  return *this;
}

logStream &logStream::operator<<(char v) {
  if (buf_.avail() >= 1) {
    buf_.append(&v, 1);
  }
  return *this;
}

logStream &logStream::operator<<(const char *v) {
  if (v) {
    size_t len = strlen(v);
    if (buf_.avail() >= len)
      buf_.append(v, len);
  } else {
    if (buf_.avail() >= 6)
      buf_.append("(null)", 6);
  }
  return *this;
}

logStream &logStream::operator<<(const Buffer &v) {
  if (buf_.avail() >= v.size()) {
    buf_.append(v.begin(), v.size());
  }
  return *this;
}

logStream &logStream::operator<<(const unsigned char *v) {
  return operator<<(reinterpret_cast<const char *>(v));
}

logStream &logStream::operator<<(const std::string &v) {
  if (buf_.avail() >= v.size()) {
    buf_.append(v, v.size());
  }
  return *this;
}

} // namespace adl