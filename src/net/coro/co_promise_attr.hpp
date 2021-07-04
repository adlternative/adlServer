#ifndef __CO_PROMISE_ATTR_H__
#define __CO_PROMISE_ATTR_H__

#ifndef __cpp_impl_coroutine
#define __cpp_impl_coroutine true
#endif

#if __cplusplus < 202002L
#undef __cplusplus
#define __cplusplus 202002L
#endif

#include <coroutine>

template <typename init, typename finish> struct promise_suspend_attr {
  init initial_suspend() { return {}; }
  finish final_suspend() noexcept { return {}; }
};

template <typename init, typename finish>
struct promise_suspend_attr_return_void {
  init initial_suspend() noexcept { return {}; }
  finish final_suspend() noexcept { return {}; }
  void return_void() noexcept {}
  // future_ get_return_object() {
  //   return {
  //       .h_ =
  //           std::coroutine_handle<typename
  //           future_::promise_type>::from_promise(
  //               *this)};
  // }
  void unhandled_exception() noexcept(false) { throw; }
};

#endif // __CO_PROMISE_ATTR_H__