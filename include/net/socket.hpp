#pragma once

#include <coroutine>
#include <memory>
#include <string_view>

#include "common/noncopyable.h"
#include "coro/task.hpp"
#include "net/event_manager.hpp"

namespace net {

class RecvAwaiter;
class SendAwaiter;

class Socket : noncopyable {
 public:
  Socket(std::string_view ip, unsigned port, EventManager &event_manager);

  Socket(Socket &&socket) noexcept
      : fd_(socket.fd_),
        io_state_(socket.io_state_),
        event_manager_(socket.event_manager_) {
    socket.fd_ = -1;
  }

  ~Socket();

  auto accept() -> coro::Task<std::shared_ptr<Socket>>;

  auto recv(void *buffer, std::size_t len) -> RecvAwaiter;

  auto send(void *buffer, std::size_t len) -> SendAwaiter;

  auto GetEventManager() const -> EventManager & { return event_manager_; }

  void SetIOState(unsigned int io_state) { io_state_ = io_state; }

  auto GetIOState() -> unsigned int { return io_state_; }

  auto GetFd() const -> int { return fd_; }

  void EventAttach() { detached_ = false; }

  auto Attached() -> bool { return !detached_; }

  void EventDetach() { detached_ = true; }

 private:
  explicit Socket(int fd, EventManager &event_manager);

 private:
  EventManager &event_manager_;
  int fd_{-1};
  unsigned int io_state_{0};  // for epoll
  bool detached_{true};
};

}  // namespace net
