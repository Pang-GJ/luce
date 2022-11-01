#pragma once

namespace coro::detail {

template <typename T>
struct Result {
  Result() = default;

  explicit Result(T &&value) : value_(value) {}

  auto Get() -> T { return value_; }

 private:
  T value_;
};

template <>
struct Result<void> {
  Result() = default;

  // do nothing
  auto Get() -> void {}
};
}  // namespace coro::detail
