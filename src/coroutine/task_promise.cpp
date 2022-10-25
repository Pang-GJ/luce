#include "coroutine/task.hpp"
#include "coroutine/task_promise.hpp"

auto TaskPromise<void>::get_return_object() -> Task<void> {
  return Task{std::coroutine_handle<TaskPromise<void>>::from_promise(*this)};
}

void TaskPromise<void>::GetResult() {
    std::unique_lock<std::mutex> lock(mtx_);
    if (!result_.has_value()) {
      completion_.wait(lock);
    }

    result_->Get();
  }

void TaskPromise<void>::unhandled_exception() {
  std::lock_guard<std::mutex> lock(mtx_);
  result_ = Result<void>(std::current_exception());
  completion_.notify_all();
  notify_callbacks();
}

void TaskPromise<void>::notify_callbacks() {
  auto value = result_.value();
  for (auto &callback : completion_callbacks_) {
    callback(value);
  }
  completion_callbacks_.clear();
}

void TaskPromise<void>::return_void() {
  std::lock_guard<std::mutex> lock(mtx_);
  result_ = Result<void>();
  completion_.notify_all();
  notify_callbacks();
}

void TaskPromise<void>::on_completed(std::function<void(Result<void>)> &&func) {
  std::unique_lock<std::mutex> lock(mtx_);
  if (result_.has_value()) {
    auto value = result_.value();
    lock.unlock();
    func(value);
  } else {
    completion_callbacks_.emplace_back(func);
  }
}
