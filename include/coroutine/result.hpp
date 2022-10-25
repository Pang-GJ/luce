#pragma once

#include <exception>

template <typename T>
struct Result {
  explicit Result() = default;

  explicit Result(T &&value) : value_(value) {}

  explicit Result(std::exception_ptr &&exception_ptr)
      : exception_ptr_(exception_ptr) {}

  /**
   * 获取Result具体内容
   * @return 有异常则抛出异常，否则返回值
   */
  T Get() {
    if (exception_ptr_) {
      std::rethrow_exception(exception_ptr_);
    }
    return value_;
  }

 private:
  T value_{};
  std::exception_ptr exception_ptr_;
};

// void特化
template <>
struct Result<void> {
  explicit Result() = default;

  explicit Result(std::exception_ptr &&exception_ptr)
      : exception_ptr_(exception_ptr) {}

  /**
   * void特化版本没有返回值，有异常则抛出异常
   */
  void Get() {
    if (exception_ptr_) {
      std::rethrow_exception(exception_ptr_);
    }
  }

 private:
  std::exception_ptr exception_ptr_;
};