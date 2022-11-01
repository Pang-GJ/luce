#pragma once

#include <condition_variable>
#include <coroutine>
#include <cstdlib>
#include <optional>

#include "common/logger.hpp"
#include "coro/task_awaiter-old.hpp"
#include "coro/task_awaiter.hpp"
#include "coro/task_promise.hpp"

namespace coro {
template <typename T = void>
struct Task {
  using promise_type = detail::PromiseType<T>;

  Task() : handle_(nullptr) {}

  explicit Task(std::coroutine_handle<promise_type> handle) : handle_(handle) {}

  //  auto await_ready() -> bool { return false; }
  //
  //  auto await_resume() -> T { return GetResult(); }
  //
  //  void await_suspend(std::coroutine_handle<> waiter) {
  //    handle_.promise().continuation_ = waiter;
  //    handle_.resume();
  //  }

  void resume() { handle_.resume(); }

  auto GetResult() -> T {
    if (!handle_.promise().is_ready()) {
      // FIXME(pgj): has bug??
      handle_.resume();
    }
    return handle_.promise().get_result();
  }

  auto operator co_await() const noexcept {
    return detail::AwaiterBase<T>{handle_.promise()};
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
