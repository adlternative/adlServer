#ifndef ASYNCLOGGING_H
#define ASYNCLOGGING_H
#include "adlBuffer.h"
#include "logStream.h"
#include <atomic>
#include <boost/noncopyable.hpp>
#include <condition_variable>
#include <future>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
// #include "
namespace adl {

class asyncLogging : boost::noncopyable {
public:
  using Buffer = adlBuffer<kLargeBuffer>;
  using BufferPtr = std::unique_ptr<Buffer>;
  using BufferVec = std::vector<std::unique_ptr<Buffer>>;
  /* 文件名，日志前后端交互时间间隔，滚动上限大小，刷磁盘的时间间隔 */
  asyncLogging(const char *logFileName, size_t writeInterval, size_t roll_size,
               size_t flushInterval);
  ~asyncLogging();
  void append(const char *str, size_t len);
  void start();

  void stop();

private:
  void threadFunc();
  std::thread td_;
  mutable std::mutex m_;
  std::condition_variable cv_;
  std::promise<void> p_;
  std::atomic<bool> running_;
  size_t roll_size_; /* 日志文件 滚动大小 超过这个大小会触发滚动 */
  size_t flushInterval_;    /* 刷新磁盘时间间隔 */
  size_t writeInterval_;    /* 前后端交流的最大间隔时间 */
  BufferPtr cur_;           /* 当前的BUF */
  BufferPtr prv_;           /* 预备的缓冲区 */
  BufferVec transBufVec_;   /* 需要传送的Buf列表 */
  std::string logFileName_; /* 日志文件名 */
};

}; // namespace adl

#endif
