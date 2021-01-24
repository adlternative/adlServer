#ifndef ADL_NON_COPYABLE_H
#define ADL_NON_COPYABLE_H
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

#endif