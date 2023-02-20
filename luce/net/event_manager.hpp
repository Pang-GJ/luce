#pragma once

#include <sys/epoll.h>
#include <cstddef>
#include <vector>

#include "luce/common/singleton.hpp"
#include "luce/coro/task.hpp"

namespace net {

class Socket;

// a epoll manager for coroutine awaitable
class EventManager {
 public:
  explicit EventManager(size_t init_size = 16);

  ~EventManager() = default;

  void Start();

  void Attach(const std::shared_ptr<Socket> &socket,
              std::coroutine_handle<> coro_handle,
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
                   unsigned int new_state, std::coroutine_handle<> coro_handle);

  int epfd_;
  bool is_shutdown_{false};
  std::vector<struct epoll_event> events_;
};

}  // namespace net
