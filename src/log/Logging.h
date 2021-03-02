#ifndef LOGGING_H
#define LOGGING_H
#include "../base/timeStamp.h"
#include "logStream.h"
#include <functional>
#include <string>

namespace adl {
class Logger {
public:
  using outFunc = std::function<void(const char *str, size_t len)>;
  using flushFunc = std::function<void()>;

  enum logLevel {
    TRACE,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL,
    NUM_LOG_LEVELS,
  };
  Logger(const char *filename, size_t line, const char *func, int savedErrno,
         logLevel level = INFO);
  ~Logger();
  logLevel getLevel() { return level_; }
  static void setglobalLevel(logLevel level);

  static void setglobalFlushFunc(flushFunc func);
  static void setglobalOutFunc(outFunc func);
  static outFunc getglobalOutFunc();
  static flushFunc getglobalFlushFunc();

  void formatTime();

  timeStamp ts_;
  logStream ls_;
  logLevel level_;
  const char *file_;
  const char *func_;
  size_t line_;
};
extern Logger::logLevel globalLogLevel;

#define LOG(level)                                                             \
  if (adl::Logger::level >= adl::globalLogLevel)                               \
  adl::Logger(__FILE__, __LINE__, __func__,                                    \
              (adl::Logger::level == adl::Logger::ERROR ||                     \
               adl::Logger::level == adl::Logger::FATAL)                       \
                  ? errno                                                      \
                  : 0,                                                         \
              adl::Logger::level)                                              \
      .ls_

#define unglyTrace(Class)                                                      \
  LOG(TRACE) << typeid(Class).name() << "::" << __func__ << adl::endl;         \
  fprintf(stderr, "%s::%s\n", typeid(Class).name(), __func__);
} // namespace adl
#endif
