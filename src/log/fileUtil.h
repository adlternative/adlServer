#ifndef FILEUTIL_H
#define FILEUTIL_H
#include <boost/noncopyable.hpp>
#include <string>

namespace adl {
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
