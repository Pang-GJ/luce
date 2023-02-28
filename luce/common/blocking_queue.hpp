#pragma once

#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <queue>

template <typename T>
class BlockingQueue {
 public:
  void push(T &&value) {
    {
      std::scoped_lock<std::mutex> lock(mtx_);
      queue_.push(std::move(value));
    }
    cond_.notify_one();
  }

  bool try_push(const T &value) {
    {
      std::unique_lock<std::mutex> lock(mtx_, std::try_to_lock);
      if (!lock) {
        return false;
      }
      queue_.push(value);
    }
    cond_.notify_one();
    return true;
  }

  bool pop(T &item) {
    std::unique_lock<std::mutex> lock(mtx_);
    cond_.wait(lock, [&]() { return !queue_.empty() || stop_; });
    if (queue_.empty()) {
      return false;
    }
    item = std::move(queue_.front());
    queue_.pop();
    return true;
  }

  // non-blocking pop an item, maybe failed
  bool try_pop_if(T &item, bool (*predict)(T &) = nullptr) {
    std::unique_lock<std::mutex> lock(mtx_, std::try_to_lock);
    if (!lock || queue_.empty()) {
      return false;
    }

    if (predict && !predict(queue_.front())) {
      return false;
    }

    item = std::move(queue_.front());
    queue_.pop();
    return true;
  }

  std::size_t size() const {
    std::scoped_lock<std::mutex> lock(mtx_);
    return queue_.size();
  }

  bool empty() const {
    std::scoped_lock<std::mutex> lock(mtx_);
    return queue_.empty();
  }

  void stop() {
    stop_ = false;
    cond_.notify_all();
  }

 private:
  mutable std::mutex mtx_;
  std::condition_variable cond_;
  std::queue<T> queue_;

  std::atomic<bool> stop_{false};
};
