#ifndef FILEUTIL_H
#define FILEUTIL_H
#include <boost/noncopyable.hpp>
#include <string>

namespace adl {

size_t xread(int fd, void *buf, size_t len);
size_t xwrite(int fd, const void *buf, size_t len);
size_t xfwrite(FILE *fp, const void *buf, size_t len);
size_t xfwrite_unlocked(FILE *fp, const void *buf, size_t len);
int xopen(const char *path, int oflag, ...);
size_t xpread(int fd, void *buf, size_t len, off_t offset);
int xdup(int fd);
FILE *xfopen(const char *path, const char *mode);
FILE *xfdopen(int fd, const char *mode);
class fileApp : boost::noncopyable {

public:
  explicit fileApp(const std::string &filename);
  ~fileApp();
  int append(const char *logLine, size_t len);
  void flush();
  off_t getWrittenBytes() { return writtenBytes_; }
  FILE *getFp() { return fp_; }

private:
  size_t write(const char *logLine, size_t len);
  char buffer_[64 * 1024];
  FILE *fp_;
  off_t writtenBytes_;
};

class fileRead {};

} // namespace adl
#endif
