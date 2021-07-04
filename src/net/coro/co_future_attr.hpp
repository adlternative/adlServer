#ifndef __CO_FUTURE_ATTR_H__
#define __CO_FUTURE_ATTR_H__

#include "co_promise_attr.hpp"
#include <coroutine>

template <typename T = int> struct read_future {
  struct promise_type;
  using handle = std::coroutine_handle<promise_type>;
  struct generator_input {};

  struct promise_type : promise_suspend_attr_return_void<std::suspend_always,
                                                         std::suspend_always> {
    T value_;
    ~promise_type() { /* 是否应该释放资源?*/
    }

    std::suspend_always yield_value(T value) {
      value_ = value;
      return {};
    }
  };
  explicit read_future(handle h = nullptr) noexcept : h_{h} {}
  handle h_;
};

template <typename T = int> struct write_future {
  struct promise_type;
  using handle = std::coroutine_handle<promise_type>;
  struct promise_type : promise_suspend_attr_return_void<std::suspend_always,
                                                         std::suspend_always> {
    T value_;
    ~promise_type() { /* 是否应该释放资源?*/
    }

    std::suspend_always yield_value(T value) {
      value_ = value;
      return {};
    }
  };
  explicit write_future(handle h = nullptr) noexcept : h_{h} {}

  handle h_;
};

struct null_future {
  struct promise_type;
  using handle = std::coroutine_handle<promise_type>;

  struct promise_type : promise_suspend_attr_return_void<std::suspend_never,
                                                         std::suspend_never> {
    //
    ~promise_type() { /* 是否应该释放资源?*/
    }

    null_future get_return_object() {
      return null_future{
          std::coroutine_handle<promise_type>::from_promise(*this)};
    }
  };

  explicit null_future(handle h = nullptr) noexcept : h_{h} {}
  handle h_;
};

#endif // __CO_FUTURE_ATTR_H__