#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <charconv>
#include <cstring>

#include "common/logger.hpp"
#include "net/async_syscall.hpp"
#include "net/socket.hpp"

namespace net {

Socket::Socket(std::string_view ip, unsigned port, IOContext &io_context)
    : io_context_(io_context) {
  struct sockaddr_in addr {};
  bzero(&addr, sizeof addr);
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = inet_addr(ip.data());

  fd_ = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC,
                 IPPROTO_TCP);
  int opt;
  // TODO(pgj): implement a function to do this.
  ::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof opt);

  if (::bind(fd_, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) != 0) {
    LOG_FATAL("bind error\n");
  }

  // TODO(pgj): don't use magic number.
  if (::listen(fd_, 8) != 0) {
    LOG_FATAL("listen error\n");
  }

  io_context_.Attach(this);
  io_context.WatchRead(this);
}

Socket::Socket(int fd, net::IOContext &io_context)
    : fd_(fd), io_context_(io_context) {
  // TODO(pgj): make a func to call
  auto flag = fcntl(fd_, F_GETFL);
  flag |= O_NONBLOCK;
  fcntl(fd_, F_SETFL, flag);
  io_context_.Attach(this);
}

Socket::~Socket() {
  if (fd_ == -1) {
    return;
  }
  io_context_.Detach(this);
  LOG_DEBUG("close fd = %d\n", fd_);
  ::close(fd_);
  // TODO(pgj): destroy the coroutine ???
  if (coro_send_) {
    coro_send_.destroy();
  }
  if (coro_recv_) {
    coro_recv_.destroy();
  }
}

auto Socket::accept() -> coro::Task<std::shared_ptr<Socket>> {
  int fd = co_await AsyncAccept{this};
  if (fd == -1) {
    LOG_FATAL("accept error\n");
  }
  co_return std::shared_ptr<Socket>(new Socket(fd, io_context_));
}

auto Socket::recv(void *buffer, std::size_t len) -> AsyncRecv {
  return {this, buffer, len};
}

auto Socket::send(void *buffer, std::size_t len) -> AsyncSend {
  return {this, buffer, len};
}

auto Socket::ResumeRecv() -> bool {
  if (!coro_recv_) {
    LOG_INFO("no handle for recv.\n");
    return false;
  }
  coro_recv_.resume();
  return true;
}

auto Socket::ResumeSend() -> bool {
  if (!coro_send_) {
    LOG_INFO("no handle for send.\n");
    return false;
  }
  coro_send_.resume();
  return true;
}

}  // namespace net
