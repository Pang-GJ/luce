#pragma once

#include <cstddef>
#include <thread>
#include <vector>
#include <functional>
#include <sys/epoll.h>
#include <memory>

#include "common/blocking_queue.hpp"
#include "coro/task.hpp"

namespace coro {

// This class is used to execute callbacks of completion entries
class Executor {
  using work_queue = BlockingQueue<struct epoll_event>;
 public:
  explicit Executor(size_t num_workers = 4);

  ~Executor();

  using callback_fn = std::function<void(const struct epoll_event&)>;
  using callback_ptr = callback_fn *;

  void ProcessEv(struct epoll_event &ev);  

  void Shutdown();

  void DestroyCallback(callback_ptr cb);
  
  template <typename T>
  static callback_ptr MakeCallback(const T &cb) {
    return new callback_fn(cb);
  }

 private:
  std::vector<std::thread> workers_; 
  work_queue work_queue_;

  void Execute(size_t worker_id);
};

}  // namespace coro
