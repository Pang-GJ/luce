#include <cstddef>
#include <cstdlib>

#include "coro/executor.hpp"
#include "common/logger.hpp"

namespace coro {

Executor::Executor(size_t num_workers) {
  for (size_t i = 0; i < num_workers; ++i) {
    workers_.emplace_back(&Executor::Execute, this, i);
  }
}

Executor::~Executor() {
  Shutdown();
  for (auto &&worker : workers_) {
    worker.join();
  }
}

void Executor::ProcessEv(struct epoll_event &ev) { work_queue_.push(ev); }

void Executor::Shutdown() { work_queue_.close(); }

void Executor::DestroyCallback(callback_ptr cb) { delete cb; }

void Executor::Execute(size_t worker_id) {
  while (true) {
    auto ev = work_queue_.pop();
    auto cb = reinterpret_cast<callback_ptr>(ev.data.ptr);
    if (cb == nullptr) {
      LOG_ERROR("%zu worker exited.", worker_id);
      exit(-1);
    }
    (*cb)(ev);
    DestroyCallback(cb);
  }
}

}  // namespace coro
