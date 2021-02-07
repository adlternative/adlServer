#ifndef UTIL_H
#define UTIL_H
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <error.h>
#include <stdarg.h>
#define NORETURN __attribute__((__noreturn__))

#define CHECK_ERR(msg)                                                         \
  {                                                                            \
    fprintf(stderr, "%s\n", msg);                                              \
    ::abort();                                                                 \
  }

#define PERROR_EXIT(msg)                                                       \
  {                                                                            \
    perror(msg);                                                               \
    ::abort();                                                                 \
  }

#define CHECK_WARN(msg)                                                        \
  {                                                                            \
    fprintf(stderr, "error:%s\n", msg);                                        \
    return -1;                                                                 \
  }

#define DEBUG_LINE_MSG(msg)                                                    \
  {                                                                            \
    fprintf(stderr, "error:%s line:%d\n", msg, __LINE__);                      \
    ::abort();                                                                 \
  }

#define DIE(msg)                                                               \
  {                                                                            \
    fprintf(stderr, "file:%s line:%d msg:%s\n", __FILE__, __LINE__, msg);      \
    ::abort();                                                                 \
  }
#define ERR(fmt, args...) fprintf(stderr, "Error: " fmt, ##args)
#endif