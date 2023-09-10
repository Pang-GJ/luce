#pragma once

#include <arpa/inet.h>
#include <asm-generic/errno-base.h>
#include <asm-generic/errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>

#include "luce/co/task.hpp"
#include "luce/common/logger.hpp"
#include "luce/net/event_manager.hpp"
#include "luce/net/socket.hpp"
#include "luce/net/tcp/tcp_acceptor.hpp"
#include "luce/net/tcp/tcp_connection.hpp"

namespace net {

namespace detail {

constexpr int HEADER_SIZE = 4;

// example only
class ReadInnerAwaiter {
 public:
  ReadInnerAwaiter(TcpConnection *conn, void *buffer, size_t len)
      : conn_(conn), buffer_(buffer), len_(len) {}

  // if IO is ready (recv_ > 0), then we should not suspend
  auto await_ready() -> bool {
    LOG_DEBUG("recv ready");
    if (!conn_->IsClosed()) {
      recv_ = ::read(conn_->GetSocket()->GetFd(), buffer_, len_);
    }
    return recv_ >= 0;
  }

  void await_suspend(std::coroutine_handle<> handle) {
    LOG_DEBUG("await_suspend recv");
    if (!conn_->IsClosed()) {
      conn_->GetEventManager().AddRecv(conn_->GetSocket(), handle);
    }
  }

  auto await_resume() -> ssize_t {
    LOG_DEBUG("recv resume");
    if (recv_ < 0) {
      if (!conn_->IsClosed()) {
        conn_->GetEventManager().DelRecv(conn_->GetSocket());
        recv_ = ::read(conn_->GetSocket()->GetFd(), buffer_, len_);
      }
    }
    return recv_;
  }

 private:
  TcpConnection *conn_;
  void *buffer_;
  ssize_t recv_{0};
  size_t len_;
};

class WriteInnerAwaiter {
 public:
  WriteInnerAwaiter(TcpConnection *conn, const void *buffer, size_t len)
      : conn_(conn), buffer_(buffer), len_(len) {}

  auto await_ready() -> bool {
    LOG_DEBUG("send ready");
    if (!conn_->IsClosed()) {
      send_ = ::write(conn_->GetSocket()->GetFd(), buffer_, len_);
    }
    return send_ >= 0;
  }

  void await_suspend(std::coroutine_handle<> handle) {
    LOG_DEBUG("send suspend");
    if (!conn_->IsClosed()) {
      conn_->GetEventManager().AddSend(conn_->GetSocket(), handle);
    }
  }

  auto await_resume() -> ssize_t {
    LOG_DEBUG("send resume");
    if (send_ < 0) {
      if (!conn_->IsClosed()) {
        conn_->GetEventManager().DelSend(conn_->GetSocket());
        send_ = ::write(conn_->GetSocket()->GetFd(), buffer_, len_);
      }
    }
    return send_;
  }

 private:
  TcpConnection *conn_;
  const void *buffer_;
  ssize_t send_{0};
  size_t len_;
};

}  // namespace detail

class AcceptAwaiter {
 public:
  explicit AcceptAwaiter(TcpAcceptor *acceptor) : acceptor_(acceptor) {}

  bool await_ready() {
    conn_fd_ = do_accept(acceptor_->GetSocket()->GetFd());
    return conn_fd_ >= 0;
  }

  void await_suspend(std::coroutine_handle<> handle) {
    LOG_DEBUG("await_suspend accept, handle: {}", handle.address());
    acceptor_->GetEventManager().AddRecv(acceptor_->GetSocket(), handle);
  }

  int await_resume() {
    LOG_DEBUG("resume accept");
    acceptor_->GetEventManager().DelRecv(acceptor_->GetSocket());
    if (conn_fd_ < 0) {
      conn_fd_ = do_accept(acceptor_->GetSocket()->GetFd());
    }
    LOG_DEBUG("accept {}", conn_fd_);
    return conn_fd_;
  }

 private:
  int do_accept(int fd) {
    struct sockaddr_in addr {};
    socklen_t len = sizeof addr;
    bzero(&addr, len);
    // 设置为非阻塞
    return ::accept4(acceptor_->GetSocket()->GetFd(), (struct sockaddr *)&addr,
                     &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
  }

  TcpAcceptor *acceptor_;
  int conn_fd_{-1};  // the coming connection fd
};

co::Task<size_t> AsyncRead(TcpConnection *conn, IOBuffer &buffer);

co::Task<size_t> AsyncWrite(TcpConnection *conn, const IOBuffer &buffer);

co::Task<bool> AsyncReadPacket(TcpConnection *conn, IOBuffer &buffer);

co::Task<bool> AsyncWritePacket(TcpConnection *conn, const IOBuffer &buffer);

}  // namespace net
