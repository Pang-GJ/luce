#pragma once

#include <coroutine>
#include <memory>
#include <string_view>

#include "common/noncopyable.h"
#include "coro/task.hpp"
#include "net/io_context.hpp"

namespace net {

class AsyncAccept;
class AsyncRecv;
class AsyncSend;
class IOContext;

class Socket : noncopyable {
 public:
  Socket(std::string_view ip, unsigned port, IOContext &io_context);

  Socket(Socket &&socket) noexcept
      : io_context_(socket.io_context_),
        fd_(socket.fd_),
        io_state_(socket.io_state_) {
    socket.fd_ = -1;
  }

  ~Socket();

  auto accept() -> coro::Task<std::shared_ptr<Socket>>;

  auto ResumeRecv() -> bool;

  auto ResumeSend() -> bool;

  auto recv(void *buffer, std::size_t len) -> AsyncRecv;

  auto send(void *buffer, std::size_t len) -> AsyncSend;

 private:
  friend AsyncAccept;
  friend AsyncRecv;
  friend AsyncSend;
  friend IOContext;

  explicit Socket(int fd, IOContext &io_context);

 private:
  IOContext &io_context_;
  int fd_{-1};
  unsigned int io_state_{0};  // for epoll
  // two coroutine
  std::coroutine_handle<> coro_recv_;  // coro for recv
  std::coroutine_handle<> coro_send_;  // coro for send
};

}  // namespace net