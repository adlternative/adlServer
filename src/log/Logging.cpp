#include "Logging.h"
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <thread>
namespace adl {

thread_local char t_errnobuf[512];
thread_local std::string t_time;
thread_local time_t t_lastSecond;
thread_local int tid = 0;
Logger::logLevel globalLogLevel;

const char *LogLevelStr[Logger::NUM_LOG_LEVELS] = {
    "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL",
};

void dftOut(const char *str, size_t len) { fwrite(str, 1, len, stdout); }
void dftFlush() { fflush(stdout); }

Logger::outFunc gOut = dftOut;
Logger::flushFunc gFlush = dftFlush;
/* 线程安全的错误函数 */
const char *strerror_tl(int savedErrno) {
  return strerror_r(savedErrno, t_errnobuf, sizeof t_errnobuf);
}

pid_t gettid() {
  /* 获得/系统内唯一的线程pid */
  return static_cast<pid_t>(::syscall(SYS_gettid));
}

Logger::Logger(const char *filename, size_t line, const char *func,
               int savedErrno, logLevel level)
    : ts_(timeStamp::now()), file_(filename), line_(line), func_(func),
      level_(level) {
  // time
  formatTime();
  // tid
  ls_ << (tid ? tid : (tid = gettid())) << " ";
  // logLevel
  ls_ << LogLevelStr[level] << " ";
  // errno
  if (savedErrno != 0) {
    ls_ << strerror_tl(savedErrno) << " (errno=" << savedErrno << ") ";
  }
}

Logger::~Logger() {
  const logStream::Buffer &buf(ls_.getBuf());
  // size_t sz = buf.size();
  gOut(buf.begin(), buf.size());
  // for (;;) {
  //   n = fwrite(buf.begin(), 1, sz, stdout);
  //   if (n == -1) {
  //     if (errno == EINTR) {
  //       continue;
  //     } else {
  //       break;
  //     }
  //   }
  //   break;
  // }
  if (level_ == adl::Logger::FATAL) {
    gFlush();
    abort();
  }
}

void Logger::setglobalLevel(logLevel level) { globalLogLevel = level; }

void Logger::setglobalOutFunc(outFunc func) { gOut = func; }

void Logger::setglobalFlashFunc(flushFunc func) { gFlush = func; }

void Logger::formatTime() {
  auto s = ts_.seconds();
  if (s != t_lastSecond) {
    t_lastSecond = s;
    t_time = std::move(ts_.toFormattedString(true));
  }
  ls_ << t_time << " ";
}

} // namespace adl