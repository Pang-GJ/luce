#pragma once

#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>

#include "common/logger.hpp"

template <typename T>
class BlockingQueue {
 public:
  void push(const T &value) {
    std::lock_guard<std::mutex> lock(mtx_);

    queue_.push(value);
    cv_.notify_one();
  }

  auto pop() -> std::optional<T> {
    std::unique_lock<std::mutex> lock(mtx_);
    cv_.wait(lock,
             [this]() { return this->closed_.test() || !queue_.empty(); });

    if (closed_.test()) {
      return {};
    }
    if (queue_.empty()) {
      LOG_ERROR("queue empty");
    }

    T value = queue_.front();
    queue_.pop();
    return value;
  }

  auto size() { return queue_.size(); }

  auto destroy() -> std::queue<T> & {
    std::lock_guard<std::mutex> lock(mtx_);
    closed_.test_and_set();
    cv_.notify_all();

    return queue_;
  }

 private:
  std::mutex mtx_;
  std::condition_variable cv_;
  std::queue<T> queue_;

  std::atomic_flag closed_;
};
