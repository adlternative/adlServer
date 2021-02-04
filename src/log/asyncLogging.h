#ifndef ASYNCLOGGING_H
#define ASYNCLOGGING_H
#include "adlBuffer.h"
#include "logStream.h"
#include <atomic>
#include <boost/noncopyable.hpp>
#include <condition_variable>
#include <future>
#include <mutex>
#include <thread>
#include <vector>
// #include "
namespace adl {

class asyncLogging : boost::noncopyable {
public:
  using Buffer = adlBuffer<kLargeBuffer>;
  using BufferPtr = std::unique_ptr<Buffer>;
  using BufferVec = std::vector<std::unique_ptr<Buffer>>;
  asyncLogging(size_t time = 3);

  ~asyncLogging();
  void append(const char *str, size_t len);
  void start();
  void flush();
  void stop();

private:
  void threadFunc();
  std::thread td_;
  mutable std::mutex m_;
  std::condition_variable cv_;
  std::promise<void> p_;
  std::atomic<bool> running_;
  size_t time_;   /* 时间 */
  BufferPtr cur_; /* 当前的BUF */
  BufferPtr prv_;
  BufferVec transBufVec_;
};

}; // namespace adl

#endif
