#pragma once

#include "common/noncopyable.h"
#include "coroutine/result.hpp"
#include "coroutine/task_awaiter.hpp"
#include "coroutine/task_promise.hpp"

template <typename ResultType>
struct Task : noncopyable {
  using promise_type = TaskPromise<ResultType>;

  explicit Task(std::coroutine_handle<promise_type> handle) noexcept
      : handle_(handle) {}

  Task(Task &&task) noexcept : handle_(std::exchange(task.handle_, {})) {}

  ~Task() {
    if (handle_) {
      handle_.destroy();
    }
  }

  auto GetResult() -> ResultType { return handle_.promise().GetResult(); }

  auto Then(std::function<void(ResultType)> &&func) -> Task & {
    handle_.promise().on_completed([func](auto result) {
      try {
        func(result.Get());
      } catch (std::exception &e) {
        // 忽略异常
      }
    });
    return *this;
  }

  auto Catching(std::function<void(std::exception &)> &&func) -> Task & {
    handle_.promise().on_completed([func](auto result) {
      try {
        // 忽略返回值，抛出异常
        result.Get();
      } catch (std::exception &e) {
        func(e);
      }
    });
    return *this;
  }

  auto finally(std::function<void()> &&func) -> Task & {
    handle_.promise().on_completed([func](auto result) { func(); });
    return *this;
  }

 private:
  std::coroutine_handle<promise_type> handle_;
};

// 还需要补充void特化版本
template <>
struct Task<void> {
  using promise_type = TaskPromise<void>;

  explicit Task(std::coroutine_handle<promise_type> handle) noexcept
      : handle_(handle) {}

  Task(Task &&task) noexcept : handle_(std::exchange(task.handle_, {})) {}

  ~Task() {
    if (handle_) {
      handle_.destroy();
    }
  }

  /**
   * 没有返回值，那么这个函数的作用就只有阻塞当前线程等待协程完成
   */
  void GetResult() { handle_.promise().GetResult(); }

  auto Then(std::function<void()> &&func) -> Task<void> & {
    handle_.promise().on_completed([func](auto result) {
      try {
        // 这里的Get只是检查有没有抛出异常
        result.Get();
        func();
      } catch (std::exception &e) {
        // ignore.
      }
    });
    return *this;
  }

  auto Catching(std::function<void(std::exception &)> &&func) -> Task<void> & {
    handle_.promise().on_completed([func](auto result) {
      try {
        result.Get();
      } catch (std::exception &e) {
        func(e);
      }
    });
    return *this;
  }

  auto finally(std::function<void()> &&func) -> Task<void> & {
    handle_.promise().on_completed([func](auto result) { func(); });
    return *this;
  }

 private:
  std::coroutine_handle<promise_type> handle_;
};