#pragma once

#include <cassert>
#include <coroutine>
#include <cstdint>
#include <memory>
#include <utility>

#include "luce/common/logger.h"
#include "luce/common/noncopyable.h"

namespace co {

template <typename T>
class Result {
 public:
  void return_value(T&& value) { value_ = T(std::move(value)); }
  void return_value(const T& value) { value_ = value; }

  std::suspend_always yield_value(T&& value) {
    value_ = T(std::move(value));
    return {};
  }

  std::suspend_always yield_value(const T& value) {
    value_ = value;
    return {};
  }

  void set_value(T&& value) { value_ = T(std::move(value)); }
  void set_value(const T& value) { value_ = value; }

  T result() { return value_; }

  T value_;
};

template <>
class Result<void> {
 public:
  void return_void() {}
  void result() {}
};

template <typename T, typename CoroHandle>
struct PromiseBase : public Result<T> {
  std::suspend_always initial_suspend() { return {}; }

  decltype(auto) final_suspend() noexcept {
    struct Awaiter {
      bool await_ready() noexcept { return false; }

      std::coroutine_handle<> await_suspend(
          CoroHandle suspended_coro) noexcept {
        // let coroutine_handle go out, it comes from await_suspend
        auto& promise = suspended_coro.promise();
        std::coroutine_handle<> continuation = promise.continuation_;
        if (promise.is_detached_) {
          LOG_DEBUG("suspended_coro.destroy();");
          suspended_coro.destroy();
        }
        return continuation;
      }

      // won't never resume
      constexpr void await_resume() const noexcept {}
    };
    return Awaiter{};
  }

  void set_continuation(std::coroutine_handle<> continuation) {
    continuation_ = continuation;
  }

  void set_thrd(int32_t thrd_id) { thrd_id_ = thrd_id; }

  void detach() noexcept { is_detached_ = true; }

  std::coroutine_handle<> continuation_{std::noop_coroutine()};
  bool is_detached_{false};
  int32_t thrd_id_{-1};
};

template <typename T = void>
struct Task : noncopyable {
  struct promise_type
      : public PromiseBase<T, std::coroutine_handle<promise_type>> {
    promise_type() = default;

    Task<T> get_return_object() {
      return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
    }

    void unhandled_exception() { LOG_FATAL("unhandled exception"); }
  };

  struct TaskAwaiterBase {
    explicit TaskAwaiterBase(std::coroutine_handle<promise_type> handle)
        : handle_(handle) {}

    bool await_ready() {
      bool res = !handle_ || handle_.done();
      LOG_DEBUG("TaskAwaitter await_ready: {}", res);
      return res;
    }

    std::coroutine_handle<> await_suspend(
        std::coroutine_handle<> continuation) noexcept {
      LOG_DEBUG("set continuation");
      handle_.promise().set_continuation(continuation);
      return handle_;
    }

    std::coroutine_handle<promise_type> handle_;
  };

  using CoroHandle = std::coroutine_handle<promise_type>;

  explicit Task(CoroHandle handle) : handle_(handle) {
    LOG_DEBUG("new Task: {}", handle.address());
  }

  Task(Task&& other) noexcept
      : handle_(std::exchange(other.handle_, nullptr)) {}

  ~Task() {
    LOG_DEBUG("~Task: {}", handle_.address());
    if (handle_) {
      handle_.destroy();
    }
  }

  auto operator co_await() const& noexcept {
    struct Awaiter : TaskAwaiterBase {
      using TaskAwaiterBase::TaskAwaiterBase;

      decltype(auto) await_resume() { return this->handle_.promise().result(); }
    };
    return Awaiter(handle_);
  }

  auto operator co_await() const&& noexcept {
    struct Awaiter : TaskAwaiterBase {
      using TaskAwaiterBase::TaskAwaiterBase;

      decltype(auto) await_resume() {
        return std::move(this->handle_.promise()).result();
      }
    };
    return Awaiter(handle_);
  }

  bool ready() const noexcept { return !handle_ || handle_.done(); }

  T result() const { return handle_.promise().result(); }

  // NOTE(pgj): 这里的实现比较粗糙，只是为了能够在 main 函数执行一个 coroutine
  T run() {
    if (ready()) {
      return result();
    }
    auto handle = handle_;
    detach();
    handle.resume();
    return handle.promise().result();
  }

  void resume() { handle_.resume(); }

  // explicit operator CoroHandle() const { return handle_; }

  CoroHandle get_handle() noexcept { return handle_; }

  void detach() noexcept {
    handle_.promise().detach();
    handle_ = nullptr;
  }

  friend void swap(Task& lhs, Task& rhs) noexcept {
    std::swap(lhs.handle_, rhs.handle_);
  }

  CoroHandle handle_;
};

}  // namespace co
