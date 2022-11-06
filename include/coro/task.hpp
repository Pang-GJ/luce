#pragma once

#include <cassert>
#include <coroutine>
#include <exception>
#include <future>
#include <utility>

#include "common/logger.hpp"

namespace coro {

// make ValueReturner to not write PromiseBase<void>
template <typename T>
class ValueReturner {
 public:
  void return_value(T &&value) { value_ = T(std::move(value)); }
  // std::promise<T> promise_;
  //
  auto GetResult() -> T { return value_; }
  T value_;
};

template <>
class ValueReturner<void> {
 public:
  void return_void() {}

  void GetResult() {}
  // std::promise<void> promise_;
};

template <typename T, typename CoroHandle>
struct PromiseBase : public ValueReturner<T> {
  auto initial_suspend() -> std::suspend_never { return {}; }

  auto final_suspend() noexcept {
    struct Awaiter {
      std::coroutine_handle<> release_detached_;

      auto await_ready() noexcept -> bool { return false; }

      auto await_suspend(CoroHandle suspended_coro) noexcept
          -> std::coroutine_handle<> {
        // let coroutine_handle go out, it comes from await_suspend
        if (suspended_coro.promise().continuation_) {
          return suspended_coro.promise().continuation_;
        }

        if (release_detached_) {
          release_detached_.destroy();
        }
        return std::noop_coroutine();
      }

      void await_resume() noexcept {}
    };

    return Awaiter{release_detached_};
  }

  std::coroutine_handle<> continuation_;
  std::coroutine_handle<> release_detached_;
};

template <typename T = void>
struct Task {
  struct promise_type
      : public PromiseBase<T, std::coroutine_handle<promise_type>> {
    promise_type() = default;

    auto get_return_object() -> Task<T> {
      return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
    }

    void unhandled_exception() {
      // this->promise_.set_exception(std::current_exception());
      // not handle exception now
      LOG_FATAL("unhandled exception");
    }

    // auto GetFuture() -> std::future<T> & { return future_; }

    void SetDetachedTask(std::coroutine_handle<promise_type> handle) {
      this->release_detached_ = handle;
    }

    // std::future<T> future_;
  };

  struct TaskAwaiter {
    explicit TaskAwaiter(std::coroutine_handle<promise_type> handle)
        : handle_(handle) {}

    auto await_ready() -> bool { return handle_.done(); }

    auto await_suspend(std::coroutine_handle<> continuation) noexcept {
      handle_.promise().continuation_ = continuation;
    }

    auto await_resume() { return handle_.promise().GetResult(); }

    std::coroutine_handle<promise_type> handle_;
  };

  using CoroHandle = std::coroutine_handle<promise_type>;

  explicit Task(CoroHandle handle) : handle_(handle) {}

  Task(Task &&other) noexcept
      : handle_(std::exchange(other.handle_, nullptr)),
        detached_(std::exchange(other.detached_, true)) {}

  ~Task() {
    if (!detached_) {
      if (!handle_.done()) {
        handle_.promise().SetDetachedTask(handle_);
        try {
          // GetFuture().get();
        } catch (std::exception &e) {
          LOG_ERROR(e.what());
        }
      } else {
        handle_.destroy();
      }
    }
  }

  auto operator co_await() const { return TaskAwaiter(handle_); }

  // auto GetFuture() const -> std::future<T> & {
  //   return handle_.promise().GetFuture();
  // }

  auto GetResult() const -> T { return handle_.promise().GetResult(); }

  void resume() { handle_.resume(); }

  explicit operator CoroHandle() const { return handle_; }

  void Detach() {
    assert(!detached_);
    handle_.promise().SetDetachedTask(handle_);
    detached_ = true;
  }

  CoroHandle handle_;
  bool detached_{};
};

}  // namespace coro
