#pragma once

#include <coroutine>
#include <optional>
#include <mutex>
#include <condition_variable>
#include <list>
#include <functional>

template <typename ResultType>
struct Task;

template <typename ResultType>
struct TaskAwaiter;

template <typename ResultType>
struct TaskPromise {
  // 协程立即执行
  std::suspend_never initial_suspend() { return {}; }

  // 执行结束后，等待外部销毁
  std::suspend_always final_suspend() noexcept { return {}; }

  // 构造协程的返回值对象 Task
  auto get_return_object() -> Task<ResultType> {
    return Task{std::coroutine_handle<TaskPromise>::from_promise(*this)};
  }

  void unhandled_exception() {
    std::lock_guard<std::mutex> lock(mtx_);
    result_ = Result<ResultType>(std::current_exception());

    // 通知GetResult()的wait
    completion_.notify_all();
    // 调用回调
    notify_callbacks();
  }

  void return_value(ResultType value) {
    // 将返回值存入result，对应于协程内部的 "co_return value"
    std::lock_guard<std::mutex> lock(mtx_);
    result_ = Result<ResultType>(std::move(value));

    // 通知GetResult()的wait
    completion_.notify_all();
    // 调用回调
    notify_callbacks();
  }

  /**
   * 获取结果
   * @return 返回结果的值
   */
  auto GetResult() -> ResultType {
    // 如果result没有值，说明协程没有运行结束，等待写入值
    std::unique_lock<std::mutex> lock(mtx_);
    if (!result_.has_value()) {
      // 等待写入值
      completion_.wait(lock);
    }
    // 如果有值，直接返回（或者抛出异常）
    return result_->Get();
  }

  // 在promise_type中定义await_transfrom可以为Task提供'co_await'支持
  // 注意这里的模板参数，不一样
  template <typename _ResultType>
  auto await_transform(Task<_ResultType> &&task) -> TaskAwaiter<_ResultType> {
    return TaskAwaiter<_ResultType>(std::move(task));
  }

  /**
   * 完成时调用回调
   */
   void on_completed(std::function<void(Result<ResultType>)> &&func) {
     std::unique_lock<std::mutex> lock(mtx_);
     if (result_.has_value()) {
       auto value = result_.value();
       lock.unlock();
       func(value);
     } else {
       // 否则添加到回调列表，等待调用
       completion_callbacks_.emplace_back(func);
     }
   }

  private:

   /**
    * 执行回调
    */
   void notify_callbacks() {
     auto value = result_.value();
     for (auto &callback : completion_callbacks_) {
       callback(value);
     }

     completion_callbacks_.clear();
   }

  // 使用optional可以判断协程是否执行完成
  std::optional<Result<ResultType>> result_;
  std::mutex mtx_;
  std::condition_variable completion_;

  // 回调列表，允许对一个Task添加多个回调
  std::list<std::function<void(Result<ResultType>)>> completion_callbacks_;
};

template<>
struct TaskPromise<void> {
  std::suspend_never initial_suspend() { return {}; }

  std::suspend_always final_suspend() noexcept { return {}; }

  auto get_return_object() -> Task<void>;

  void GetResult();

  void unhandled_exception();

  void return_void();

  void on_completed(std::function<void(Result<void>)> &&func);

 private:
  std::optional<Result<void>> result_;
  std::mutex mtx_;
  std::condition_variable completion_;

  std::list<std::function<void(Result<void>)>> completion_callbacks_;

  void notify_callbacks();
};