#define CHECK_ERR(msg)                                                         \
  {                                                                            \
    fprintf(stderr, "%s\n", msg);                                              \
    exit(1);                                                                   \
  }
#define PERROR_EXIT(msg)                                                       \
  {                                                                            \
    perror(msg);                                                               \
    exit(1);                                                                   \
  }
#define CHECK_WARN(msg)                                                        \
  {                                                                            \
    fprintf(stderr, "error:%s\n", msg);                                        \
    return -1;                                                                 \
  }
#define DEBUG_LINE_MSG(msg)                                                    \
  {                                                                            \
    fprintf(stderr, "error:%s line:%d\n", msg, __LINE__);                      \
    exit(1);                                                                   \
  }