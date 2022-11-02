#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>

#include "common/logger.hpp"

template <typename T>
class BlockingQueue {
 public:
  void push(const T &value) {
    std::lock_guard<std::mutex> lock(mtx_);
    if (closed_) {
      LOG_ERROR("queue have been closed.");
    }
    queue_.push(value);
    cv_.notify_one();
  }

  auto pop() -> T {
    std::unique_lock<std::mutex> lock(mtx_);
    cv_.wait(lock, [this]() { return !queue_.empty() || closed_; });

    if (queue_.empty()) {
      LOG_ERROR("queue empty");
    }

    T value = queue_.front();
    queue_.pop();
    return value;
  }

  auto size() { return queue_.size(); }

  void close() {
    std::lock_guard<std::mutex> lock(mtx_);
    closed_ = true;
    cv_.notify_all();
  }

 private:
  std::mutex mtx_;
  std::condition_variable cv_;
  std::queue<T> queue_;
  bool closed_{false};
};