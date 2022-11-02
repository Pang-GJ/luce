#pragma once

#include <cstddef>

#include "common/singleton.hpp"
#include "coro/task.hpp"

namespace net {

class Socket;

// a epoll manager for coroutine awaitable
// singleton
class EventManager {
 public:
  explicit EventManager(size_t init_size = 16);

  // ~EventManager() { Singleton<EventManager>::DestroyInstance(); }
  ~EventManager() = default;

  void Start();

  void Attach(Socket *socket);
  void Detach(Socket *socket);

  void AddRecv(Socket *socket, std::coroutine_handle<> recv_coro);
  void DelRecv(Socket *socket);

  void AddSend(Socket *socket, std::coroutine_handle<> send_coro);
  void DelSend(Socket *socket);

 private:
  void UpdateEvent(Socket *socket, unsigned int new_state, std::coroutine_handle<> coro_handle);

  int epfd_;
  size_t init_size_;
};

}  // namespace net
