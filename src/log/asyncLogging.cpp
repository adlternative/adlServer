#include "asyncLogging.h"
#include "../util.h"
#include "timeStamp.h"
#include <functional>
#include <iostream>
namespace adl {
asyncLogging::asyncLogging(size_t time)
    : time_(time), td_(std::bind(&asyncLogging::threadFunc, this)),
      cur_(new Buffer), prv_(new Buffer) {}

asyncLogging::~asyncLogging() { flush(); }

void asyncLogging::append(const char *str, size_t len) {
  if (!running_)
    DIE("haven't running");
  if (len >= kLargeBuffer)
    DIE("overflow: size >= kLargeBuffer");
  std::unique_lock<std::mutex> u(m_);
  /* 如果当前的buf还没满，继续填 */
  /* 正常情况 */
  if (cur_->avail() >= len + 1) {
    cur_->append(str, len);
  } else {
    /* 否则将buf放入传送给后端的向量里面 */
    transBufVec_.push_back(std::move(cur_));
    /* 如果预留的buf不是空的 */
    if (prv_)
      cur_ = std::move(prv_);
    else {
      /* 否则 重新分配内存给当前buf*/
      cur_.reset(new Buffer);
    }
    /* 添加buf到现在的buf */
    if (cur_->avail() >= len + 1)
      cur_->append(str, len);
    /* 唤醒后端 */
    cv_.notify_one();
  }
}

void asyncLogging::start() {
  std::future<void> f = p_.get_future();
  running_.store(true, std::memory_order_release);
  f.get();
}

void asyncLogging::flush() { fflush(stdout); }

void asyncLogging::threadFunc() {
  // 等待start
  while (!running_.load(std::memory_order_acquire))
    ;
  // 表示就绪
  p_.set_value();
  BufferPtr newBuf_1(new Buffer), newBuf_2(new Buffer);
  BufferVec needToWriteBufVec_;
  needToWriteBufVec_.reserve(16);
  while (running_.load()) {
    {
      std::unique_lock<std::mutex> u(m_);
      /* 等待超时或者有buf送来 */
      cv_.wait_for(u, std::chrono::seconds(time_),
                   [this]() { return !transBufVec_.empty(); });
      /* 将cur填入 */
      transBufVec_.push_back(std::move(cur_));
      /* 补给cur_,prv */
      cur_ = std::move(newBuf_1);
      if (!prv_)
        prv_ = std::move(newBuf_2);
      /* 交换需要写的bufVec和运输的bufVec,
      这时候需要写的bufVec是空的 */
      needToWriteBufVec_.swap(transBufVec_);
    }
    /* 如果这次的数据过载 */
    if (needToWriteBufVec_.size() >= 25) {
      char buf[256];
      snprintf(buf, sizeof buf,
               "Dropped log messages at %s, %lu larger buffers\n",
               timeStamp::now().toFormattedString().c_str(),
               needToWriteBufVec_.size() - 2);
      // todo:DIE();
      fputs(buf, stderr);
      // TO LOGFILE
      needToWriteBufVec_.resize(2);
    }
    // fopen("a.log",t);
    for (const auto &i : needToWriteBufVec_) {
      fwrite(i->begin(), 1, i->size(), stdout);
    }
    if (needToWriteBufVec_.size() > 2)
      needToWriteBufVec_.resize(2);
    /* 如果newbufs都用完了，
      用需写缓冲区向量中的最后两个缓冲区填充 */
    if (!newBuf_1) {
      newBuf_1 = std::move(needToWriteBufVec_.back());
      needToWriteBufVec_.pop_back();
      /* 注意这个reset是作为清空缓冲区 */
      newBuf_1->reset();
    }
    if (!newBuf_2) {
      newBuf_2 = std::move(needToWriteBufVec_.back());
      needToWriteBufVec_.pop_back();
      newBuf_2->reset();
    }
    flush();
  }
  flush();
}

} // namespace adl