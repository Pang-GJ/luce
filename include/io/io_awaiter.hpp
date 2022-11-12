#pragma once

#include <arpa/inet.h>
#include <sys/types.h>
#include <cstring>

#include "common/logger.hpp"
#include "net/event_manager.hpp"
#include "net/socket.hpp"
#include "net/tcp_acceptor.hpp"
#include "net/tcp_connection.hpp"

namespace net {

// example only
class ReadAwaiter {
 public:
  ReadAwaiter(TcpConnection *conn, void *buffer, size_t len)
      : conn_(conn), buffer_(buffer), len_(len) {}

  // if IO is ready (recv_ > 0), then we should not suspend
  auto await_ready() -> bool {
    LOG_DEBUG("recv ready");
    recv_ = ::read(conn_->GetSocket()->GetFd(), buffer_, len_);
    return recv_ >= 0;
  }

  void await_suspend(std::coroutine_handle<> handle) {
    LOG_DEBUG("recv suspend");
    conn_->GetEventManager().AddRecv(conn_->GetSocket(), handle);
  }

  auto await_resume() -> ssize_t {
    LOG_DEBUG("recv resume");
    if (recv_ < 0) {
      conn_->GetEventManager().DelRecv(conn_->GetSocket());
      recv_ = ::read(conn_->GetSocket()->GetFd(), buffer_, len_);
    }
    return recv_;
  }

 private:
  TcpConnection *conn_;
  void *buffer_;
  ssize_t recv_{0};
  size_t len_;
};

class WriteAwaiter {
 public:
  WriteAwaiter(TcpConnection *conn, void *buffer, size_t len)
      : conn_(conn), buffer_(buffer), len_(len) {}

  auto await_ready() -> bool {
    LOG_DEBUG("send ready");
    send_ = ::write(conn_->GetSocket()->GetFd(), buffer_, len_);
    return send_ >= 0;
  }

  void await_suspend(std::coroutine_handle<> handle) {
    LOG_DEBUG("send suspend");
    conn_->GetEventManager().AddSend(conn_->GetSocket(), handle);
  }

  auto await_resume() -> ssize_t {
    LOG_DEBUG("send resume");
    if (send_ < 0) {
      conn_->GetEventManager().DelSend(conn_->GetSocket());
      send_ = ::write(conn_->GetSocket()->GetFd(), buffer_, len_);
    }
    return send_;
  }

 private:
  TcpConnection *conn_;
  void *buffer_;
  ssize_t send_{0};
  size_t len_;
};

class AcceptAwaiter {
 public:
  explicit AcceptAwaiter(TcpAcceptor *acceptor) : acceptor_(acceptor) {}

  auto await_ready() -> bool {
    conn_fd_ = do_accept(acceptor_->GetSocket()->GetFd());
    return conn_fd_ >= 0;
  }

  void await_suspend(std::coroutine_handle<> handle) {
    LOG_DEBUG("suspend accept");
    acceptor_->GetEventManager().AddRecv(acceptor_->GetSocket(), handle);
  }

  auto await_resume() -> int {
    LOG_DEBUG("resume accept");
    acceptor_->GetEventManager().DelRecv(acceptor_->GetSocket());
    if (conn_fd_ < 0) {
      conn_fd_ = do_accept(acceptor_->GetSocket()->GetFd());
    }
    LOG_DEBUG("accept %d", conn_fd_);
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

}  // namespace net
