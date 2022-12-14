#pragma once

#include "common/blocking_queue.hpp"
#include "common/logger.hpp"
#include "coro/task.hpp"

#include <thread>

class CoroThreadPool {
 public:
  explicit CoroThreadPool(size_t thread_num) {
    for (size_t i = 0; i < thread_num; ++i) {
      workers_.emplace_back([this] { this->work(); });
    }
  }

  ~CoroThreadPool() {
    auto &queue = tasks_.destroy();
    for (auto &worker : workers_) {
      if (worker.joinable()) {
        worker.join();
      }
    }


    while (!queue.empty()) {
      // queue.front().destroy();
      queue.pop();
    }
  }

  void SubmitCoroHandle(std::coroutine_handle<> task) { tasks_.push(task); }

 private:
  void work() {
    while (auto task = tasks_.pop()) {
      task.value().resume();
    }
  }

  std::vector<std::thread> workers_;
  BlockingQueue<std::coroutine_handle<>> tasks_;
};
