#ifndef LOGFILE_H
#define LOGFILE_H
#include "fileUtil.h"
#include "timeStamp.h"
#include <boost/noncopyable.hpp>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
namespace adl {
class logFile : boost::noncopyable {
public:
  logFile(const std::string basename, off_t rollSize, int flushInterval = 3,
          int checkEveryN = 1024)
      : baseName_(basename), rollSize_(rollSize), flushInterval_(flushInterval),
        checkEveryN_(checkEveryN), count_(0), mutex_(new std::mutex),
        startOfPeriod_(0), lastRoll_(0), lastFlush_(0) {

    rollfile(); /* 第一次总得生成一个文件给我们去写吧！ */
  }

  ~logFile() = default;
  void append(const char *logline, size_t len) {
    if (mutex_) {
      std::unique_lock<std::mutex> lock(*mutex_);
      append_unlocked(logline, len);
    }
  }
  void flush() {
    if (mutex_) {
      std::unique_lock<std::mutex> lock(*mutex_);
      file_->flush();
    }
  }
  /* 滚动：生成新的文件 */
  bool rollfile() {
    time_t now = 0;
    /* 首先是最基础的文件名 */
    std::string filename(baseName_);
    /* 文件名添加时间 可以加进程信息 暂时不加*/
    filename += logTime(baseName_, &now);
    /* 后缀 */
    filename += ".log";
    /* 获得1970年以后到现在的“天数” */
    time_t start = now / kRollPerSeconds_ * kRollPerSeconds_;
    /* 如果当前时间还小于上次滚动的时间，感觉就是bug */
    if (now <= lastRoll_)
      return false;
    /* 更新最后一次滚动的时间和天数*/
    lastRoll_ = now;
    lastFlush_ = now;
    startOfPeriod_ = start;
    /* 以后开始写到新文件名的文件 */
    file_.reset(new fileApp(filename));
    return true;
  }
  std::string logTime(const std::string &baseName, time_t *now) {
    *now = time(NULL);
    return timeStamp::getYearToSecondFormatTime(now);
  }

private:
  void append_unlocked(const char *logline, int len) {

    file_->append(logline, len); /* 向文件添加内容 */
    /* 如果日志文件写入的大小已经超过了我们指定的滚动大小 */
    if (file_->getWrittenBytes() > rollSize_) {
      /* 滚动 */
      rollfile();
    } else {
      ++count_;
      if (count_ >= checkEveryN_) {
        count_ = 0;
        time_t now = ::time(NULL);
        time_t thisPeriod_ = now / kRollPerSeconds_ * kRollPerSeconds_;
        if (thisPeriod_ != startOfPeriod_) {
          /* 过一天 滚日志 */
          rollfile();
        } else if (now - lastFlush_ > flushInterval_) {
          /* 超时刷磁盘 */
          lastFlush_ = now;
          file_->flush();
        }
      }
    }
  }
  // static string getLogFileName(const std::string&s);
  const std::string baseName_;
  const off_t rollSize_;    /* 超过滚动大小将触发滚动 */
  const int flushInterval_; /* 超过刷新间隔将触发刷新到磁盘 */
  const int checkEveryN_; /* append的次数count_ 超过 checkEveryN_ 将检验时间
                             是否应触发滚动 */
  int count_;             /* append的次数 */

  std::unique_ptr<std::mutex> mutex_;
  time_t startOfPeriod_;
  time_t lastRoll_;  /* 最后一次滚动的时间 */
  time_t lastFlush_; /* 最后一次刷新的时间 */
  std::unique_ptr<fileApp> file_;
  const static int kRollPerSeconds_ = 60 * 60 * 24; // one day
};
} // namespace adl
#endif
