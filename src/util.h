#ifndef UTIL_H
#define UTIL_H
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <error.h>
#include <stdarg.h>
#include <thread>
#define NORETURN __attribute__((__noreturn__))

#define CHECK_ERR(msg)                                                         \
  {                                                                            \
    fprintf(stderr, "%s", msg);                                                \
    ::abort();                                                                 \
  }

#define PERROR_EXIT(msg)                                                       \
  {                                                                            \
    perror(msg);                                                               \
    ::abort();                                                                 \
  }

#define WARN_(msg)                                                             \
  {                                                                            \
    fprintf(stderr, "[WARN]:%s", msg);                                         \
    return -1;                                                                 \
  }

#define DEBUG_(msg)                                                            \
  {                                                                            \
    fprintf(stderr, "[DEBUG] file:%s line:%d msg:%s\n", __FILE__, __LINE__,    \
            msg);                                                              \
  }

#define DIE(msg)                                                               \
  {                                                                            \
    fprintf(stderr, "[DIE]: file:%s line:%d msg:%s", __FILE__, __LINE__, msg); \
    ::abort();                                                                 \
  }

/* 非线程安全 */
#define DIE_WITH_ERRNO_STR(msg)                                                \
  {                                                                            \
    fprintf(stderr, "[DIE]: %s %s", msg, strerror(errno));                     \
    ::abort();                                                                 \
  }
#define WARN_WITH_ERRNO_STR(msg)                                               \
  {                                                                            \
    fprintf(stderr, "[WARN]: %s %s", msg, strerror(errno));                    \
    return -1;                                                                 \
  }

#define INFO_(fmt, args...) fprintf(stdout, "[INFO]:" fmt, ##args)
#define ERR_(fmt, args...) fprintf(stderr, "[ERR]:" fmt, ##args)

#define ERROR_WITH_ERRNO_STR(msg)                                              \
  { fprintf(stderr, "[ERROR]: %s %s", msg, strerror(errno)); }

template <typename To, typename From> inline To implicit_cast(From const &f) {
  return f;
}

#endif