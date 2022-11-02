#pragma once

#include <coroutine>
#include <memory>
#include <string_view>

#include "common/noncopyable.h"
#include "coro/task.hpp"
#include "net/event_manager.hpp"
// #include "net/io_context.hpp"
//#include "net/io_awaiter.hpp"

namespace net {

class AsyncAccept;
class AsyncRecv;
class AsyncSend;
class IOContext;

class RecvAwaiter;
class SendAwaiter;

class Socket : noncopyable {
 public:
  Socket(std::string_view ip, unsigned port, EventManager &event_manager);

  Socket(Socket &&socket) noexcept
      :         fd_(socket.fd_),
        io_state_(socket.io_state_), event_manager_(socket.event_manager_) {
    socket.fd_ = -1;
  }

  ~Socket();

  auto accept() -> coro::Task<std::shared_ptr<Socket>>;

  // auto ResumeRecv() -> bool;

  // auto ResumeSend() -> bool;

  auto recv(void *buffer, std::size_t len) -> RecvAwaiter;

  auto send(void *buffer, std::size_t len) -> SendAwaiter;

  // void SetCoroRecv(std::coroutine_handle<> handle) { coro_recv_ = handle; }

  // void SetCoroSend(std::coroutine_handle<> handle) { coro_send_ = handle; }

  // auto GetIOContext() const -> IOContext & { return io_context_; }
   auto GetEventManager() const -> EventManager & { return event_manager_; }

   void SetIOState(unsigned int io_state) {
     io_state_ = io_state;
   }

   auto GetIOState() -> unsigned int {
     return io_state_;
   }

  auto GetFd() const -> int { return fd_; }

 private:
  // friend IOContext;
//  friend EventManager;

  explicit Socket(int fd, EventManager &event_manager);

 private:
  // IOContext &io_context_;
  EventManager &event_manager_;
  int fd_{-1};
  unsigned int io_state_{0};  // for epoll
  // two coroutine
  // std::coroutine_handle<> coro_recv_;  // coro for recv
  // std::coroutine_handle<> coro_send_;  // coro for send
};

}  // namespace net
