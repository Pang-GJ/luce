#pragma once

#include <coroutine>
#include <cstdlib>

namespace coro {
template <typename T>
struct Task;

namespace detail {
template <typename T>
struct PromiseTypeBase {
  std::coroutine_handle<> continuation_ = std::noop_coroutine();

  auto initial_suspend() -> std::suspend_always { return {}; }

  struct FinalAwaiter {
    auto await_ready() noexcept -> bool { return false; }

    void await_resume() noexcept {}

    template <typename PromiseType>
    auto await_suspend(std::coroutine_handle<PromiseType> coro) noexcept
        -> std::coroutine_handle<> {
      return coro.promise().continuation_;
    }
  };

  // TODO(pgj): what the return type is???
  auto final_suspend() noexcept { return FinalAwaiter{}; }

  void unhandled_exception() {
    // TODO(pgj): how to handle the exception?
    exit(-1);
  }

  //  auto get_return_object() -> Task<T>;
};

template <typename T>
struct PromiseType : PromiseTypeBase<T> {
  T result_;

  void return_value(T value) { result_ = value; }

  auto get_return_object() -> Task<T>;
};

template <>
struct PromiseType<void> : PromiseTypeBase<void> {
  void return_void() {}
  void await_resume() {}
  auto get_return_object() -> Task<void>;
};

}  // namespace detail

template <typename T = void>
struct Task {
  using promise_type = detail::PromiseType<T>;

  Task() : handle_(nullptr) {}

  explicit Task(std::coroutine_handle<promise_type> handle) : handle_(handle) {}

  auto await_ready() -> bool { return false; }

  auto await_resume() -> T { return handle_.promise().result_; }

  void await_suspend(std::coroutine_handle<> waiter) {
    handle_.promise().continuation_ = waiter;
    handle_.resume();
  }

  void resume() { handle_.resume(); }

  auto GetResult() -> T {
    return handle_.promise().result_;
  }

  std::coroutine_handle<promise_type> handle_;
};

namespace detail {
template <typename T>
inline auto PromiseType<T>::get_return_object() -> Task<T> {
  return Task<T>{std::coroutine_handle<PromiseType<T>>::from_promise(*this)};
}

inline auto PromiseType<void>::get_return_object() -> Task<void> {
  return Task<void>{
      std::coroutine_handle<PromiseType<void>>::from_promise(*this)};
}

}  // namespace detail

}  // namespace coro