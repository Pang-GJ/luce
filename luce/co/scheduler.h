#pragma once

#include <utility>
#include "luce/co/co_thread_pool.h"
#include "luce/co/task.hpp"
#include "luce/common/singleton.hpp"
namespace co {

class Scheduler {
 public:
  Scheduler() noexcept = default;

  void co_spawn(Task<> &&task) noexcept;

 private:
  co::ThreadPool tp_;
};

void co_spawn(Task<> &&task) noexcept {
  Singleton<Scheduler>::GetInstance()->co_spawn(std::forward<Task<>>(task));
}

}  // namespace co