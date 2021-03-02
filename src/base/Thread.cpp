#include "Thread.h"
#include <cassert>
#include <string>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

namespace adl {

std::atomic<int> Thread::numCreated_(0);
Thread::Thread(ThreadFunc &&func, std::string &&name)
    : started_(false), joined_(false), func_(func), name_(name), pthreadId_(0) {
  numCreated_++;
}
Thread::~Thread() {}
class threadData {
public:
  typedef std::function<void()> ThreadFunc;
  threadData(ThreadFunc &&func, std::string &&name, std::promise<void> &p,
             pid_t *tid)
      : func_(func), name_(name), p_(p), tid_(tid) {}

  ThreadFunc func_;
  std::string name_;
  pid_t *tid_;
  std::promise<void> &p_;
};

// pid_t gettid() {
//   /* 获得/系统内唯一的线程pid */
//   return static_cast<pid_t>(::syscall(SYS_gettid));
// }

void *runThread(void *arg) {
  threadData *td = (threadData *)arg;
  *td->tid_ = gettid();
  td->p_.set_value();
  try {
    td->func_();
  } catch (const std::exception &e) {
    fprintf(stderr, "%s\n", e.what());
    abort();
  }
  return NULL;
}

void Thread::start() {
  std::future<void> f = p_.get_future();
  threadData *td =
      new threadData(std::move(func_), std::move(name_), p_, &tid_);
  if (pthread_create(&pthreadId_, NULL, runThread, td)) {
    delete (td);
    // LOG
  } else {
    started_ = true;
    f.get();
    assert(tid_ > 0);
  }
}
int Thread::join() {
  assert(!joined_);
  return pthread_join(pthreadId_, NULL);
}

} // namespace adl