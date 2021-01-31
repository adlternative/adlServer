#include "timeStamp.h"
#include <inttypes.h>
#include <stdio.h>
#include <sys/time.h>

timeStamp timeStamp::now() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  int64_t seconds = tv.tv_sec;
  return timeStamp(seconds * s2ms_ + tv.tv_usec);
}
std::string timeStamp::toString() const {
  char buf[32] = {0};
  int64_t seconds = ms_ / s2ms_;      //秒
  int64_t microseconds = ms_ % s2ms_; //剩余微秒
  snprintf(buf, sizeof(buf), "%" PRId64 ".%06" PRId64 "", seconds,
           microseconds); // 8字节的int的格式化方式
  return buf;
}

std::string timeStamp::toFormattedString(bool showMicroseconds) const {
  char buf[64] = {0};
  time_t seconds = static_cast<time_t>(ms_ / s2ms_); //秒数
  struct tm tm_time;
  gmtime_r(&seconds, &tm_time); // time_t转换成tm,该函数返回指向 tm 结构的指针，
                                //该结构带有被填充的时间信息。年月日...
                                //不过有些地方比较特殊,日从1～31,月从0～11...
  /* 是否显示微秒 */
  if (showMicroseconds) {
    int microseconds = static_cast<int>(ms_ % s2ms_);
    snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d.%06d",
             tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
             tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec, microseconds);
  } else {
    snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d",
             tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
             tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
  }
  return buf;
}