#include "fileUtil.h"
#include "../util.h"
#include "Logging.h"
#include <fcntl.h>
#include <functional>
#include <unistd.h>
namespace adl {

size_t xread(int fd, void *buf, size_t len) {
  size_t nr;
  while (1) {
    nr = ::read(fd, buf, len);
    if (nr < 0) {
      if (errno == EINTR) {
        continue;
      }
      /* EAGAIN  EWOULDBLOCK */
    }
  }
  return nr;
}

size_t xwrite(int fd, const void *buf, size_t len) {
  size_t nr;
  while (1) {
    nr = ::write(fd, buf, len);
    if (nr < 0) {
      if (errno == EINTR)
        continue;
      /* EAGAIN  EWOULDBLOCK */
    }

    return nr;
  }
}

size_t xfwrite(FILE *fp, const void *buf, size_t len) {
  size_t nr;
  while (1) {
    nr = ::fwrite(buf, 1, len, fp);
    if (nr < 0) {
      if (errno == EINTR)
        continue;
      /* EAGAIN  EWOULDBLOCK */
    }
    return nr;
  }
}

size_t xfwrite_unlocked(FILE *fp, const void *buf, size_t len) {
  size_t nr;
  while (1) {
    nr = ::fwrite_unlocked(buf, 1, len, fp);
    if (nr < 0) {
      if (errno == EINTR)
        continue;
      /* EAGAIN  EWOULDBLOCK */
    }
    return nr;
  }
}

int xopen(const char *path, int oflag, ...) {
  mode_t mode = 0;
  va_list ap;

  va_start(ap, oflag);
  if (oflag & O_CREAT)
    mode = va_arg(ap, int);
  va_end(ap);
  for (;;) {
    int fd = open(path, oflag, mode);
    if (fd >= 0)
      return fd;
    if (errno == EINTR)
      continue;
    if ((oflag & O_RDWR) == O_RDWR)
      fprintf(stderr, "could not open '%s' for reading and writing\n", path);
    else if ((oflag & O_WRONLY) == O_WRONLY)
      fprintf(stderr, "could not open '%s' for writing\n", path);
    else
      fprintf(stderr, "could not open '%s' for reading\n", path);
  }
}

size_t xpread(int fd, void *buf, size_t len, off_t offset) {
  size_t nr;
  while (1) {
    nr = pread(fd, buf, len, offset);
    if ((nr < 0) && (errno == EAGAIN || errno == EINTR))
      continue;
    return nr;
  }
}

int xdup(int fd) {
  int ret = dup(fd);
  if (ret < 0)
    fprintf(stderr, "dup failed");
  return ret;
}

FILE *xfopen(const char *path, const char *mode) {
  for (;;) {
    FILE *fp = fopen(path, mode);
    if (fp)
      return fp;
    if (errno == EINTR)
      continue;

    if (*mode && mode[1] == '+')
      fprintf(stderr, "could not open '%s' for reading and writing", path);
    else if (*mode == 'w' || *mode == 'a')
      fprintf(stderr, "could not open '%s' for writing", path);
    else
      fprintf(stderr, "could not open '%s' for reading", path);
  }
}

FILE *xfdopen(int fd, const char *mode) {
  FILE *stream = fdopen(fd, mode);
  if (stream == NULL)
    fprintf(stderr, "Out of memory? fdopen failed");
  return stream;
}

fileApp::fileApp(const std::string &filename)
    : fp_(::fopen(filename.c_str(), "ae")), writtenBytes_(0) {
  if (!fp_) {
    PERROR_EXIT("fileApp():");
  }
  ::setbuffer(fp_, buffer_, sizeof(buffer_));
}

fileApp::~fileApp() { ::fclose(fp_); }

int fileApp::append(const char *logLine, size_t len) {
  const char *p = logLine;
  size_t total = 0;
  while (len > 0) {
    size_t written = this->write(p, len);
    if (written < 0) {
      return -1;
    } else if (written == 0) {
      errno = ENOSPC;
      return -1;
    }
    len -= written;
    total += written;
    p += written;
    writtenBytes_ += written;
  }
  return total;
}

void fileApp::flush() { ::fflush(fp_); }

/* 不加锁的写 */
size_t fileApp::write(const char *logLine, size_t len) {
  return adl::xfwrite_unlocked(fp_, logLine, len);
}
} // namespace adl