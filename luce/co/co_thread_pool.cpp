#include "luce/co/co_thread_pool.h"
#include <sched.h>
#include <cassert>
#include <cstdlib>
#include <thread>
#include <vector>
#include "luce/common/logger.h"

namespace co {

namespace {

void GetCurrentCpus(std::vector<uint32_t>* ids) {
  cpu_set_t set;
  ids->clear();
  if (sched_getaffinity(0, sizeof(set), &set) == 0) {
    for (uint32_t i = 0; i < CPU_SETSIZE; ++i) {
      if (CPU_ISSET(i, &set)) {
        ids->emplace_back(i);
      }
    }
  }
}

}  // namespace

ThreadPool::ThreadPool(size_t thread_num, bool enable_work_steal,
                       bool enable_core_bindings)
    : thread_num_(thread_num != 0U ? thread_num
                                   : std::thread::hardware_concurrency()),
      task_queues_(thread_num),
      stopped_(false),
      enable_core_bindings_(enable_core_bindings),
      enable_work_steal_(enable_work_steal) {
  auto worker = [this](size_t id) {
    auto* current = GetCurrent();
    current->first = id;
    current->second = this;
    while (true) {
      TaskItem task_item{};
      if (enable_work_steal_) {
        // 首先尝试去 work steal
        for (auto i = 0; i < thread_num_; ++i) {
          if (task_queues_[(id + i) % thread_num_].try_pop_if(
                  &task_item, [](auto& item) { return item.can_steal; })) {
            break;
          }
        }
      }

      if (!task_item.handle && !task_queues_[id].pop(&task_item)) {
        // 如果一个线程已经停止，不要再等待新任务
        // 否则等待 pop 如果不允许抢占，或者抢占失败
        if (stopped_) {
          break;
        }
        continue;
      }

      if (task_item.handle) {
        task_item.handle.resume();
      }
    }
  };

  workers_.reserve(thread_num_);

  // 获取可用的 CPU
  std::vector<uint32_t> cpu_ids;
  if (enable_core_bindings_) {
    GetCurrentCpus(&cpu_ids);
  }
  const auto cpu_num = cpu_ids.size();

  for (auto i = 0; i < thread_num_; ++i) {
    workers_.emplace_back(worker, i);

    if (!enable_core_bindings_) {
      continue;
    }

    // run threads per core
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu_ids[i % cpu_num], &cpuset);
    int res = sched_setaffinity(static_cast<int>(workers_[i].native_handle()),
                                sizeof(cpu_set_t), &cpuset);
    if (res != 0) {
      LOG_ERROR("error calling sched_setaffinity: {}", res);
    }
  }
}

ThreadPool::~ThreadPool() {
  stopped_ = true;
  for (auto& queue : task_queues_) {
    queue.stop();
  }
  for (auto& worker : workers_) {
    worker.join();
  }
}

void ThreadPool::ScheduleById(TaskItem::TaskType coro, int32_t id) {
  if (nullptr == coro) {
    return;
  }
  if (stopped_) {
    return;
  }
  if (id == -1) {
    if (enable_work_steal_) {
      // try to push to a non-block queue firstly
      TaskItem task_item{coro, true};
      for (auto i = 0; i < thread_num_ * 2; ++i) {
        if (task_queues_.at(i % thread_num_).try_push(task_item)) {
          return;
        }
      }
    }

    id = rand() % thread_num_;
    task_queues_[id].push(TaskItem{coro, enable_work_steal_});
  } else {
    assert(id < thread_num_);
    task_queues_[id].push(TaskItem{coro, false});
  }
}

int32_t ThreadPool::GetCurrentId() const {
  auto* current = GetCurrent();
  if (this == current->second) {
    return current->first;
  }
  return -1;
}

size_t ThreadPool::GetItemCount() const {
  size_t res = 0;
  for (auto i = 0; i < thread_num_; ++i) {
    res += task_queues_.at(i).size();
  }
  return res;
}

std::pair<size_t, ThreadPool*>* ThreadPool::GetCurrent() const {
  static thread_local std::pair<size_t, ThreadPool*> current(-1, nullptr);
  return &current;
}

}  // namespace co