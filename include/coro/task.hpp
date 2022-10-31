#pragma once

#include <condition_variable>
#include <coroutine>
#include <cstdlib>
#include <optional>

#include "common/logger.hpp"

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

  auto final_suspend() noexcept -> FinalAwaiter { return {}; }

  void unhandled_exception() {
    // if found exception, terminate
    exit(-1);
  }

  //  auto get_return_object() -> Task<T>;
};

template <typename T>
struct PromiseType : PromiseTypeBase<T> {
  void return_value(T value) {
    std::lock_guard<std::mutex> lock(mtx_);
    result_ = std::move(T(value));
    is_ready_.notify_all();
  }

  auto get_return_object() -> Task<T>;

  auto get_result() -> T {
    std::unique_lock<std::mutex> lock(mtx_);
    if (!is_ready()) {
      is_ready_.wait(lock);
    }
    return result_.value();
  }

  auto is_ready() -> bool { return result_.has_value(); }

 private:
  std::optional<T> result_;
  std::mutex mtx_;
  std::condition_variable is_ready_;
};

template <>
struct PromiseType<void> : PromiseTypeBase<void> {
  void return_void() {
    std::lock_guard<std::mutex> lock(mtx_);
    done_ = true;
    is_ready_.notify_all();
  }
  //  void await_resume() {}
  auto get_return_object() -> Task<void>;

  void get_result() {
    std::unique_lock<std::mutex> lock(mtx_);
    if (!done_) {
      is_ready_.wait(lock);
    }
  }

  auto is_ready() -> bool { return done_; }

 private:
  bool done_{false};
  std::mutex mtx_;
  std::condition_variable is_ready_;
};

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

}  // namespace detail

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

// #define Lazy<T> Task<T, detail::AwaiterNever<T>>

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
