#pragma once

#include <sys/epoll.h>
#include <cstddef>
#include <memory>
#include <vector>

#include "luce/common/singleton.hpp"
#include "luce/common/thread_pool.hpp"
#include "luce/coro/task.hpp"

namespace net {

class Socket;

// a epoll manager for coroutine awaitable
class EventManager {
 public:
  explicit EventManager(std::shared_ptr<ThreadPool> work_thread_pool, size_t init_size = 16);

  ~EventManager() = default;

  void Start();

  void Attach(const std::shared_ptr<Socket> &socket,
              unsigned int events = EPOLLIN | EPOLLET);

  void Detach(const std::shared_ptr<Socket> &socket);

  void AddRecv(const std::shared_ptr<Socket> &socket,
               std::coroutine_handle<> recv_coro);
  void DelRecv(const std::shared_ptr<Socket> &socket);

  void AddSend(const std::shared_ptr<Socket> &socket,
               std::coroutine_handle<> send_coro);
  void DelSend(const std::shared_ptr<Socket> &socket);

  void Shutdown() { is_shutdown_ = true; }

 private:
  void UpdateEvent(const std::shared_ptr<Socket> &socket,
                   unsigned int new_state);
  int epfd_;
  std::atomic<bool> is_shutdown_{false};
  std::vector<struct epoll_event> events_;
  std::shared_ptr<ThreadPool> work_thread_pool_;
};

}  // namespace net
