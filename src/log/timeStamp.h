#ifndef TIMESTAMP_H
#define TIMESTAMP_H

#include <boost/operators.hpp>
#include <string>
class timeStamp : public boost::equality_comparable<timeStamp>,
                  public boost::less_than_comparable<timeStamp> {
public:
  timeStamp() : ms_(0) {}
  explicit timeStamp(int64_t ms) : ms_(ms) {}
  timeStamp(const timeStamp &ts) : ms_(ts.ms_) {}
  timeStamp(timeStamp &&ts) noexcept : ms_(std::move(ts.ms_)) {}
  void operator=(const timeStamp &ts) { ms_ = ts.ms_; }
  void operator=(const timeStamp &&ts) noexcept { ms_ = std::move(ts.ms_); }
  void swap(timeStamp &that) { std::swap(ms_, that.ms_); }
  std::string toString() const;
  std::string toFormattedString(bool showMicroseconds = true) const;
  /* 合法时间应>0 */
  bool valid() const { return ms_ > 0; }
  /* 作为私有成员ms_的get接口 */
  int64_t microSeconds() const { return ms_; }
  /* 从微秒转换成秒 /100000 */
  time_t seconds() const { return static_cast<time_t>(ms_ / s2ms_); }
  /* 获取当前时间 */
  static timeStamp now();

  /* 返回一个非法值：0 */
  static timeStamp invalid() { return timeStamp(); }
  /* 从秒变成微秒*/
  static timeStamp fromUnixTime(time_t t) { return fromUnixTime(t, 0); }
  /* 从秒变成微秒 加上一个指定的微秒值*/
  static timeStamp fromUnixTime(time_t s, int ms) {
    return timeStamp(static_cast<int64_t>(s) * s2ms_ + ms);
  }

  static const int s2ms_ = 1000 * 1000;

private:
  int64_t ms_;
};
inline bool operator<(timeStamp lhs, timeStamp rhs) {
  return lhs.microSeconds() < rhs.microSeconds();
}

inline bool operator==(timeStamp lhs, timeStamp rhs) {
  return lhs.microSeconds() == rhs.microSeconds();
}
/* 差多少秒 */
inline double timeDifference(timeStamp high, timeStamp low) {
  int64_t diff = high.microSeconds() - low.microSeconds();
  return static_cast<double>(diff) / timeStamp::s2ms_;
}
/* 将参数的时间戳添加一定秒数得到结果时间戳 */
inline timeStamp addTime(timeStamp timestamp, double seconds) {
  int64_t delta = static_cast<int64_t>(seconds * timeStamp::s2ms_);
  return timeStamp(timestamp.microSeconds() + delta);
}

#endif
