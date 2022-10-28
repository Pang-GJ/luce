#include <sys/epoll.h>
#include <cerrno>

#include "common/logger.hpp"
#include "net/io_context.hpp"
#include "net/socket.hpp"

namespace net {

IOContext::IOContext() : fd_(epoll_create1(EPOLL_CLOEXEC)) {
  if (fd_ == -1) {
    LOG_FATAL("epoll_create1 error\n");
  }
}

void IOContext::Run() {
  struct epoll_event events[max_events];

  while (true) {
    int nfds = epoll_wait(fd_, events, max_events, -1);
    if (nfds == -1) {
      LOG_FATAL("epoll_wait error");
    }

    for (int i = 0; i < nfds; ++i) {
      auto socket = static_cast<Socket *>(events[i].data.ptr);
      // TODO(pgj): other case to check!!
      if ((events[i].events & EPOLLIN) != 0U) {
        socket->ResumeRecv();
      } else if ((events[i].events & EPOLLOUT) != 0U) {
        socket->ResumeSend();
      }
    }
  }
}

void IOContext::Attach(net::Socket *socket) {
  struct epoll_event ev {};
  auto io_state = EPOLLIN | EPOLLET;
  ev.events = io_state;
  ev.data.ptr = socket;
  if (epoll_ctl(fd_, EPOLL_CTL_ADD, socket->fd_, &ev) == -1) {
    LOG_FATAL("epoll_ctl: attach error!\n");
  }
  socket->io_state_ = io_state;
}

void IOContext::Detach(net::Socket *socket) {
  if (epoll_ctl(fd_, EPOLL_CTL_DEL, socket->fd_, nullptr) == -1) {
    LOG_FATAL("epoll_ctl: del error!\n");
  }
}

void IOContext::UpdateState(Socket *socket, unsigned int new_state) {
  if (socket->io_state_ != new_state) {
    struct epoll_event ev {};
    ev.events = new_state;
    ev.data.ptr = socket;
    if (epoll_ctl(fd_, EPOLL_CTL_MOD, socket->fd_, &ev) == -1) {
      LOG_INFO("errno num: %d\n", errno);
      LOG_FATAL("epoll_ctl: mod error!\n");
    }
    socket->io_state_ = new_state;
  }
}

void IOContext::WatchRead(net::Socket *socket) {
  auto new_state = socket->io_state_ | EPOLLIN;
  UpdateState(socket, new_state);
}

void IOContext::UnWatchRead(net::Socket *socket) {
  auto new_state = socket->io_state_ & ~EPOLLIN;
  UpdateState(socket, new_state);
}

void IOContext::WatchWrite(net::Socket *socket) {
  auto new_state = socket->io_state_ | EPOLLOUT;
  UpdateState(socket, new_state);
}

void IOContext::UnWatchWrite(net::Socket *socket) {
  auto new_state = socket->io_state_ & ~EPOLLOUT;
  UpdateState(socket, new_state);
}

}  // namespace net