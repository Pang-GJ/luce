#pragma once

#include <condition_variable>
#include <coroutine>
#include <cstdlib>
#include <functional>
#include <list>
#include <memory>
#include <mutex>
#include <optional>

#include "coro/result.hpp"
#include "coro/task_awaiter.hpp"

namespace coro {

template <typename T>
struct Task;

namespace detail {

template <typename T>
struct TaskPromise {
  std::suspend_never initial_suspend() { return {}; }

  std::suspend_always final_suspend() noexcept { return {}; }

  auto get_return_object() -> Task<T> {
    return Task{std::coroutine_handle<TaskPromise>::from_promise(*this)};
  }

  template <typename ResultType>
  auto await_transform(Task<ResultType> &&task) -> TaskAwaiter<ResultType> {
    return {std::move(task)};
  }

  void unhandled_exception() {
    // fixme: handle exception
    std::exit(-1);
  }

  void return_value(T value) {
    std::unique_lock<std::mutex> lock(mtx_);
    result_ = Result<T>(std::move(value));
    is_completed_.notify_all();

    lock.unlock(); 
    do_callbacks();
  }

  auto get_result() -> T {
    std::unique_lock<std::mutex> lock(mtx_);
    if (!result_.has_value()) {
      is_completed_.wait(lock);
    }
    return result_->Get();
  }

  void on_completed(std::function<void(Result<T>)> &&func) {
    std::unique_lock<std::mutex> lock(mtx_);
    if (result_.has_value()) {
      auto value = result_.value();
      lock.unlock();
      func(value);
    } else {
      callbacks_.emplace_back(func);
    }
  }

 private:
  
  void do_callbacks() {
    std::list<std::function<void(Result<T>)>> temp_callbacks;
    {
      std::lock_guard<std::mutex> lock(mtx_);
      // swap with a empty list can decrease the lock comptetion
      temp_callbacks.swap(callbacks_);
    }

    for (auto &callback : temp_callbacks) {
      callback(result_);
    }
  }

  std::optional<Result<T>> result_;
  std::mutex mtx_;
  std::condition_variable is_completed_;
  std::list<std::function<void(Result<T>)>> callbacks_;
};

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
}  // namespace detail

}  // namespace coro
