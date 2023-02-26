#include <sys/epoll.h>
#include <cerrno>
#include <cstring>
#include <vector>

#include "luce/common/logger.hpp"
#include "luce/net/event_manager.hpp"
#include "luce/net/socket.hpp"

namespace net {

EventManager::EventManager(size_t init_size)
    : epfd_(epoll_create1(EPOLL_CLOEXEC)), events_(init_size) {
  if (epfd_ == -1) {
    LOG_FATAL("epoll_create1 error");
  }
}

void EventManager::Start() {
  while (true) {
    if (is_shutdown_) {
      break;
    }
    LOG_DEBUG("epoll_wait");
    int event_num = epoll_wait(epfd_, &*events_.begin(),
                               static_cast<int>(events_.size()), -1);
    if (event_num == -1) {
      if (errno == EINTR) {
        continue;
      }
      LOG_ERROR("epoll_wait error, info: {}", strerror(errno));
      LOG_FATAL("epoll_wait error");
    }

    if (event_num == static_cast<int>(events_.size())) {
      events_.resize(events_.size() * 2);
    }

    for (int i = 0; i < event_num; ++i) {
      // TODO(pgj): check more situation
      if ((events_[i].events & EPOLLIN) != 0U) {
        auto handle = static_cast<Socket::Handle *>(events_[i].data.ptr);
        auto recv_coro = handle->recv_coro;
        LOG_DEBUG("epoll_await resume recv handle");
        recv_coro.resume();

      } else if ((events_[i].events & EPOLLOUT) != 0U) {
        auto handle = static_cast<Socket::Handle *>(events_[i].data.ptr);
        auto send_coro = handle->send_coro;
        LOG_DEBUG("epoll_await resume send handle");
        send_coro.resume();
      }
    }
  }
}

void EventManager::AddRecv(const std::shared_ptr<Socket> &socket,
                           std::coroutine_handle<> recv_coro) {
  if (is_shutdown_) {
    return;
  }
  if (!socket->Attached()) {
    auto events = EPOLLIN | EPOLLET;
    Attach(socket, events);
  }
  socket->SetRecvCoro(recv_coro);
  auto new_state = socket->GetIOState() | EPOLLIN;
  socket->SetIOState(new_state);
  UpdateEvent(socket, new_state);
}

void EventManager::DelRecv(const std::shared_ptr<Socket> &socket) {
  if (is_shutdown_) {
    return;
  }
  socket->DelRecvCoro();
  auto new_state = socket->GetIOState() & ~EPOLLIN;
  socket->SetIOState(new_state);
  UpdateEvent(socket, new_state);
}

void EventManager::AddSend(const std::shared_ptr<Socket> &socket,
                           std::coroutine_handle<> send_coro) {
  if (is_shutdown_) {
    return;
  }
  if (!socket->Attached()) {
    auto events = EPOLLOUT | EPOLLET;
    Attach(socket, events);
  }
  socket->SetSendCoro(send_coro);
  auto new_state = socket->GetIOState() | EPOLLOUT;
  socket->SetIOState(new_state);
  UpdateEvent(socket, new_state);
}

void EventManager::DelSend(const std::shared_ptr<Socket> &socket) {
  if (is_shutdown_) {
    return;
  }
  socket->DelSendCoro();
  auto new_state = socket->GetIOState() & ~EPOLLOUT;
  socket->SetIOState(new_state);
  UpdateEvent(socket, new_state);
}

void EventManager::Attach(const std::shared_ptr<Socket> &socket,
                          unsigned int events) {
  struct epoll_event ev {};
  socket->SetIOState(events);
  socket->EventAttach();
  ev.events = events;
  ev.data.ptr = &socket->handle;
  if (epoll_ctl(epfd_, EPOLL_CTL_ADD, socket->GetFd(), &ev) == -1) {
    LOG_FATAL("epoll_ctl_add: add attach error!\n");
  }
}

void EventManager::Detach(const std::shared_ptr<Socket> &socket) {
  socket->EventDetach();
  if (epoll_ctl(epfd_, EPOLL_CTL_DEL, socket->GetFd(), nullptr) == -1) {
    LOG_FATAL("epoll_ctl_del: detach error!");
  }
}

void EventManager::UpdateEvent(const std::shared_ptr<Socket> &socket,
                               unsigned int new_state) {
  struct epoll_event ev {};
  ev.events = new_state;
  ev.data.ptr = &socket->handle;
  if (epoll_ctl(epfd_, EPOLL_CTL_MOD, socket->GetFd(), &ev) == -1) {
    LOG_FATAL("epoll_ctl_mod: error");
  }
}

}  // namespace net
