#include "currentThread.h"
#include "../headFile.h"
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>
namespace adl {
__thread int CurrentThread::t_cachedTid = 0;

pid_t gettid() {
  /* 获得/系统内唯一的线程pid */
  return static_cast<pid_t>(::syscall(SYS_gettid));
}

void CurrentThread::cacheTid() {
  /* 如果当前线程号是0,
    那么调用gettid获取当前线程号 */
  if (t_cachedTid == 0) {
    t_cachedTid = adl::gettid();
  }
} // namespace CurrentThread

} // namespace adl