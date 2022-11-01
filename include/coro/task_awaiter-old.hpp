#pragma once

#include "coro/task_promise.hpp"

namespace coro::detail {

template <typename T>
struct AwaiterBase {
  explicit AwaiterBase(detail::PromiseType<T> &promise) : promise_(promise) {}

  auto await_ready() noexcept -> bool { return promise_.is_ready(); }

  virtual auto await_suspend(std::coroutine_handle<> continuation)
      const noexcept -> std::coroutine_handle<> {
    promise_.continuation_ = continuation;
    return std::coroutine_handle<detail::PromiseType<T>>::from_promise(
        promise_);
  }

  auto await_resume() -> T { return promise_.get_result(); }

  detail::PromiseType<T> &promise_;
};

template <typename T>
struct AwaiterResume : AwaiterBase<T> {
  void await_suspend(std::coroutine_handle<> continuation) const noexcept {
    this->promise_.continuation_ = continuation;
    this->promise_.get_return_object().resume();
  }
};

}  // namespace coro::detail
