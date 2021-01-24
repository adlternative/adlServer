#include "currentThread.h"

namespace adl {
__thread int CurrentThread::t_cachedTid = 0;

void CurrentThread::cacheTid() {
  /* 如果当前线程号是0,
    那么调用gettid获取当前线程号 */
  if (t_cachedTid == 0) {
    t_cachedTid = adl::gettid();
  }
} // namespace CurrentThread

} // namespace adl