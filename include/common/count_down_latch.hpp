#pragma once

#include <condition_variable>
#include <cstdint>
#include <mutex>

// CountDownLatch 是个计数器锁，用于阻塞当前线程，等待其他前程都执行完毕
// 再继续执行。
class CountDownLatch {
 public:
  explicit CountDownLatch(int32_t count) : count_(count) {}

  void Wait() {
    std::unique_lock<std::mutex> lock(mtx_);

    while (count_ > 0) {
      condition_.wait(lock);
    }
  }

  void CountDown() {
    std::lock_guard<std::mutex> lock(mtx_);

    --count_;
    if (count_ == 0) {
      condition_.notify_one();
    }
  }

  auto GetCount() const -> int32_t {
    std::unique_lock<std::mutex> lock(mtx_);
    return count_;
  }

 private:
  int32_t count_{0};
  mutable std::mutex mtx_;
  std::condition_variable condition_;
};
