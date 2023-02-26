#pragma once

#include <coroutine>
#include <memory>
#include <string_view>

#include "luce/common/noncopyable.h"
#include "luce/coro/task.hpp"
#include "luce/net/event_manager.hpp"
#include "luce/net/inet_address.hpp"

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

  void SetRecvCoro(std::coroutine_handle<> read_coro) {
    handle.recv_coro = read_coro;
  }

  void DelRecvCoro() { handle.recv_coro = nullptr; }

  void SetSendCoro(std::coroutine_handle<> write_coro) {
    handle.send_coro = write_coro;
  }

  void DelSendCoro() { handle.send_coro = nullptr; }

  struct Handle {
    std::coroutine_handle<> recv_coro;
    std::coroutine_handle<> send_coro;
  };
  Handle handle;

 private:
  int fd_;
  unsigned int io_state_{0};  // for epoll
  bool detached_{true};
};

}  // namespace net
