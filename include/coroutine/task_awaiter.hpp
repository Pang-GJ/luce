#pragma once

#include <coroutine>
#include <utility>
#include "common/noncopyable.h"

template <typename ResultType>
struct Task;

template <typename Result>
struct TaskAwaiter : noncopyable {
  /**
   * 当一个 Task 实例被 co_await 时，意味着它在 co_await
   * 表达式返回之前已经执行完毕，当 co_await 表达式返回时，Task
   * 的结果也就被取到，Task 实例在后续就没有意义了。因此 TaskAwaiter
   * 的构造器当中接收 Task &&，防止 co_await 表达式之后继续对 Task 进行操作
   */
  explicit TaskAwaiter(Task<Result> &&task) noexcept : task_(std::move(task)) {}

  TaskAwaiter(TaskAwaiter<Result> &&task_awaiter) noexcept
      : task_(std::exchange(task_awaiter.task_, {})) {}

  constexpr bool await_ready() noexcept {
    // TODO(pgj): 优化点：判断协程是否已经结束
    // 直接return false，肯定会挂起
    return false;
  }

  void await_suspend(std::coroutine_handle<> handle) noexcept {
    // 当task执行完之后调用resume
    task_.finally([handle]() { handle.resume(); });
  }

  auto await_resume() noexcept -> Result { return task_.GetResult(); }

 private:
  Task<Result> task_;
};
