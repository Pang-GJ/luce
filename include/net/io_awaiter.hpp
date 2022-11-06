#pragma once

#include <arpa/inet.h>
#include <sys/types.h>

#include "common/logger.hpp"
#include "net/event_manager.hpp"
 #include "net/socket.hpp"

namespace net {

// example only
class RecvAwaiter {
 public:
  RecvAwaiter(Socket *socket, void *buffer, size_t len)
      : socket_(socket), buffer_(buffer), len_(len) {}

  // if IO is ready (recv_ > 0), then we should not suspend
  auto await_ready() -> bool {
    LOG_DEBUG("recv ready");
    recv_ = ::read(socket_->GetFd(), buffer_, len_);
    return recv_ >= 0;
  }

  void await_suspend(std::coroutine_handle<> handle) {
    LOG_DEBUG("recv suspend");
    socket_->GetEventManager().AddRecv(socket_, handle);
  }

  auto await_resume() -> ssize_t {
    LOG_DEBUG("recv resume");
    if (recv_ < 0) {
      socket_->GetEventManager().DelRecv(socket_);
      recv_ = ::read(socket_->GetFd(), buffer_, len_);
    }
    return recv_;
  }

 private:
  Socket *socket_;
  void *buffer_;
  ssize_t recv_{0};
  size_t len_;
};

class SendAwaiter {
 public:
  SendAwaiter(Socket *socket, void *buffer, size_t len)
      : socket_(socket), buffer_(buffer), len_(len) {}

  auto await_ready() -> bool {
    LOG_DEBUG("send ready");
    send_ = ::write(socket_->GetFd(), buffer_, len_);
    return send_ >= 0;
  }

  void await_suspend(std::coroutine_handle<> handle) {
    LOG_DEBUG("send suspend");
    socket_->GetEventManager().AddSend(socket_, handle);
  }

  auto await_resume() -> ssize_t {
    LOG_DEBUG("send resume");
    if (send_ < 0) {
      socket_->GetEventManager().DelSend(socket_);
      send_ = ::write(socket_->GetFd(), buffer_, len_);
    }
    return send_;
  }

 private:
  Socket *socket_;
  void *buffer_;
  ssize_t send_{0};
  size_t len_;
};

class AcceptAwaiter {
 public:
  explicit AcceptAwaiter(Socket *socket) : socket_(socket) {}

  // accept will block
  auto await_ready() -> bool {
    struct sockaddr_in addr {};
    socklen_t len = sizeof addr;
    conn_fd_ = ::accept(socket_->GetFd(), (struct sockaddr *)&addr, &len);
    return conn_fd_ >= 0;
  }

  void await_suspend(std::coroutine_handle<> handle) {
    LOG_DEBUG("suspend accept");
    socket_->GetEventManager().AddRecv(socket_, handle);
  }

  auto await_resume() -> int {
    LOG_DEBUG("resume accept");
    socket_->GetEventManager().DelRecv(socket_);
    if (conn_fd_ < 0) {
      struct sockaddr_in addr {};
      socklen_t len = sizeof addr;
      conn_fd_ = ::accept(socket_->GetFd(), (struct sockaddr *)&addr, &len);
    }
    LOG_INFO("accept %d", conn_fd_);
    return conn_fd_;
  }

 private:
  Socket *socket_;
  int conn_fd_{-1};  // the coming connection fd
};

}  // namespace net
