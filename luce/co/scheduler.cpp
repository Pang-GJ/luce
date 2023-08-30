#include "luce/co/scheduler.h"
#include "luce/co/co_thread_pool.h"

namespace co {

void Scheduler::co_spawn(Task<> &&task) noexcept {
  auto handle = task.get_handle();
  task.detach();
  tp_.ScheduleById(handle);
}

}  // namespace co