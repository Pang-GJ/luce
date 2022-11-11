#pragma once

#include <coroutine>
#include <memory>
#include <string_view>

#include "common/noncopyable.h"
#include "coro/task.hpp"
#include "net/event_manager.hpp"
#include "net/inet_address.hpp"

namespace net {

class Socket : noncopyable {
 public:
  explicit Socket(int sock_fd = -1) : fd_(sock_fd) {}
  ~Socket();

  void BindAddress(const InetAddress &local_addr);

  void Listen();

  void ShutdownWrite();

  void SetTcpNoDelay(bool on);
  void SetReuseAddr(bool on);
  void SetReusePort(bool on);
  void SetKeepAlive(bool on);
  void SetNonblock();

  void SetIOState(unsigned int io_state) { io_state_ = io_state; }

  auto GetIOState() -> unsigned int { return io_state_; }

  auto GetFd() const -> int { return fd_; }

  void EventAttach() { detached_ = false; }

  auto Attached() -> bool { return !detached_; }

  void EventDetach() { detached_ = true; }

 private:
  int fd_;
  unsigned int io_state_{0};  // for epoll
  bool detached_{true};
};

}  // namespace net
