#pragma once

#include <arpa/inet.h>
#include <cerrno>
#include <coroutine>

#include "common/logger.hpp"
#include "net/io_context.hpp"
#include "net/socket.hpp"

namespace net {

// implement async syscall
template <typename Syscall, typename ReturnType>
class AsyncSyscall {
 public:
  AsyncSyscall() = default;

  auto await_ready() const noexcept -> bool { return false; }

  auto await_suspend(std::coroutine_handle<> handle) noexcept -> bool {
    static_assert(std::is_base_of_v<AsyncSyscall, Syscall>);
    handle_ = handle;
    value_ = static_cast<Syscall *>(this)->Syscall();
    suspend_ = value_ == -1 && (errno == EAGAIN || errno == EWOULDBLOCK);
    if (suspend_) {
      static_cast<Syscall *>(this)->SetCoroHandle();
    }
    return suspend_;
  }

  auto await_resume() noexcept -> ReturnType {
    if (suspend_) {
      value_ = static_cast<Syscall *>(this)->Syscall();
    }
    return value_;
  }

 protected:
  bool suspend_{false};
  std::coroutine_handle<> handle_;
  ReturnType value_{};
};

class AsyncAccept : public AsyncSyscall<AsyncAccept, int> {
 public:
  explicit AsyncAccept(Socket *socket) : AsyncSyscall{}, socket_(socket) {
    socket_->io_context_.WatchRead(socket_);
    LOG_DEBUG("socket accept operation \n");
  }

  ~AsyncAccept() {
    socket_->io_context_.UnWatchRead(socket_);
    LOG_DEBUG("~socket accept operation \n");
  }

  int Syscall() {
    struct sockaddr_in addr {};
    socklen_t addr_len = sizeof addr;
    LOG_DEBUG("accept %d\n", socket_->fd_);
    return ::accept(socket_->fd_, (struct sockaddr *)&addr, &addr_len);
  }

  void SetCoroHandle() { socket_->coro_recv_ = this->handle_; }

 private:
  Socket *socket_;
};

class AsyncSend : public AsyncSyscall<AsyncSend, ssize_t> {
 public:
  AsyncSend(Socket *socket, void *buffer, std::size_t len)
      : AsyncSyscall{}, socket_(socket), buffer_(buffer), len_(len) {
    socket_->io_context_.WatchWrite(socket_);
    LOG_DEBUG("socket send operation\n");
  }

  ~AsyncSend() {
    socket_->io_context_.UnWatchWrite(socket_);
    LOG_DEBUG("~socket send operation\n");
  }

  ssize_t Syscall() {
    LOG_DEBUG("send fd: %d\n", socket_->fd_);
    return ::send(socket_->fd_, buffer_, len_, 0);
  }

  void SetCoroHandle() { socket_->coro_send_ = this->handle_; }

 private:
  Socket *socket_;
  void *buffer_;
  std::size_t len_;
};

class AsyncRecv : public AsyncSyscall<AsyncRecv, ssize_t> {
 public:
  AsyncRecv(Socket *socket, void *buffer, std::size_t len)
      : AsyncSyscall{}, socket_(socket), buffer_(buffer), len_(len) {
    socket_->io_context_.WatchRead(socket_);
    LOG_DEBUG("socket recv operation\n");
  }

  ~AsyncRecv() {
    socket_->io_context_.UnWatchRead(socket_);
    LOG_DEBUG("~socket recv operation\n");
  }

  ssize_t Syscall() {
    LOG_DEBUG("recv fd: %d\n", socket_->fd_);
    return ::recv(socket_->fd_, buffer_, len_, 0);
  }

  void SetCoroHandle() { socket_->coro_recv_ = handle_; }

 private:
  Socket *socket_;
  void *buffer_;
  std::size_t len_;
};

}  // namespace net