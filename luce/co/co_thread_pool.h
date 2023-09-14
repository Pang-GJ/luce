#pragma once

#include <sched.h>
#include <atomic>
#include <coroutine>
#include <cstddef>
#include <functional>
#include <thread>
#include <vector>
#include "luce/common/blocking_queue.h"

namespace co {

class ThreadPool {
 public:
  struct TaskItem {
    using TaskType = std::coroutine_handle<>;

    TaskType handle{nullptr};
    bool can_steal{false};
  };

  explicit ThreadPool(size_t thread_num = std::thread::hardware_concurrency(),
                      bool enable_work_steal = false,
                      bool enable_core_bindings = false);

  ~ThreadPool();

  void ScheduleById(TaskItem::TaskType coro, int32_t id = -1);

  int32_t GetCurrentId() const;
  size_t GetItemCount() const;
  size_t GetThreadNum() const { return thread_num_; };

 private:
  std::pair<size_t, ThreadPool*>* GetCurrent() const;
  size_t thread_num_;

  std::vector<BlockingQueue<TaskItem>> task_queues_;
  std::vector<std::thread> workers_;

  std::atomic<bool> stopped_;
  bool enable_core_bindings_;
  bool enable_work_steal_;
};

}  // namespace co