#ifndef __NONCOPYABLE_H__
#define __NONCOPYABLE_H__

namespace adl {

class noncopyable {
public:
  noncopyable(const noncopyable &) = delete;
  void operator=(const noncopyable &) = delete;

protected:
  noncopyable() = default;
  ~noncopyable() = default;
};
} // namespace adl

#endif // __NONCOPYABLE_H__