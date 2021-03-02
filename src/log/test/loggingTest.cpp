#include "Logging.h"
#include "../base/Singleton.hpp"
#include "asyncLogging.h"
#include <functional>
#include <iostream>
adl::asyncLogging *g_asyncLog = nullptr;
// void asyncOutput(const char *msg, size_t len) { g_asyncLog->append(msg, len); }
// void asyncFlash() { g_asyncLog->flush(); }
int main(int argc, char const *argv[]) {
  g_asyncLog = new adl::asyncLogging("test", 3, 1024 * 1024, 3);
  adl::Logger::setglobalLevel(adl::Logger::TRACE);
  // adl::Logger::setglobalOutFunc(asyncOutput);
  // adl::Logger::setglobalFlushFunc(asyncFlash);
  g_asyncLog->start();

  int a = 1;
  int *p = &a;
  LOG(TRACE) << "TRACE" << 1.23 << 312 << p << '\n';
  LOG(INFO) << "INFO\n";
  LOG(WARN) << "WARN\n";
  // errno = EINTR;
  LOG(ERROR) << "ERROR\n";
  while (1)
    ;
  // LOG(FATAL) << "FATAL\n";
  // LOG(DEBUG) << "hello\n";
  return 0;
}
