#pragma once

#include <utility>

#include "common/noncopyable.h"
#include "coro/task.hpp"

namespace coro {

template<typename T>
struct Task;

namespace detail {

template<typename T>
struct TaskAwaiter : noncopyable {
  explicit TaskAwaiter(Task<T> &&task) noexcept : task_(std::move(task)) {}

  TaskAwaiter(TaskAwaiter<T> &&complete_awaiter)  noexcept : task_(std::exchange(complete_awaiter.task_, {})) {}

  auto await_ready() -> bool { return false; }

  void await_suspend(std::coroutine_handle<> handle) noexcept {
    task_.finally([handle] {
      handle.resume();
    });
  }

  auto await_resume() noexcept -> T { return task_.GetResult(); }

 private:
  coro::Task<T> task_;
};

}  // namespace detail

}  // namespace coro
