#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <charconv>
#include <cstring>

#include "common/logger.hpp"
#include "net/event_manager.hpp"
#include "net/io_awaiter.hpp"
#include "net/socket.hpp"

namespace net {

Socket::Socket(std::string_view ip, unsigned port, EventManager &event_manager)
    : event_manager_(event_manager) {
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
}

Socket::Socket(int fd, EventManager &event_manager)
    : fd_(fd), event_manager_(event_manager) {
  // TODO(pgj): make a func to call
  auto flag = fcntl(fd_, F_GETFL);
  flag |= O_NONBLOCK;
  fcntl(fd_, F_SETFL, flag);
}

Socket::~Socket() {
  if (fd_ == -1) {
    return;
  }
  if (!detached_) {
    event_manager_.Detach(this);
  }
  LOG_DEBUG("close fd = %d\n", fd_);
  ::close(fd_);
}

auto Socket::accept() -> coro::Task<std::shared_ptr<Socket>> {
  int fd = co_await AcceptAwaiter{this};
  // struct sockaddr_in addr {};
  // socklen_t len = sizeof addr;
  // int fd = ::accept(fd_, (struct sockaddr *)&addr, &len);
  if (fd != -1) {
    LOG_DEBUG("accept %d", fd);
  }
  co_return std::shared_ptr<Socket>(new Socket(fd, event_manager_));
}

auto Socket::read(void *buffer, std::size_t len) -> ReadAwaiter {
  return {this, buffer, len};
}

auto Socket::write(void *buffer, std::size_t len) -> WriteAwaiter {
  return {this, buffer, len};
}

}  // namespace net
