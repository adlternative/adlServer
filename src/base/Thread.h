
#ifndef THREAD_H
#define THREAD_H
#include "noncopyable.h"
#include <atomic>
#include <functional>
#include <future>
#include <string>
#include <thread>
namespace adl {

class Thread : noncopyable {
  typedef std::function<void()> ThreadFunc;

public:
  Thread(ThreadFunc &&func, std::string &&name);
  ~Thread();
  void start();                             /* 线程启动 */
  int join();                               // return pthread_join()
  bool started() const { return started_; } /* 线程是否已经启动 */
  pid_t tid() const { return tid_; }        /* 返回线程号 */
  static int numCreated() {
    return numCreated_.load();
  } /* 有多少个线程创建了? */

private:
  bool started_;
  bool joined_;
  pthread_t pthreadId_;
  ThreadFunc func_;
  std::string name_;
  pid_t tid_;
  static std::atomic<int> numCreated_;
  std::promise<void> p_;
};
pid_t gettid();
} // namespace adl
#endif
