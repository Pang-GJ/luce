#include <sys/epoll.h>
#include <vector>

#include "common/logger.hpp"
#include "net/event_manager.hpp"
#include "net/socket.hpp"

namespace net {

EventManager::EventManager(size_t init_size)
    : epfd_(epoll_create1(EPOLL_CLOEXEC)), init_size_(init_size) {
  if (epfd_ == -1) {
    LOG_FATAL("epoll_create1 error");
  }
}

// TODO(pgj): use a thread_pool or coroutine_pool???
void EventManager::Start() {
  std::vector<struct epoll_event> events(init_size_);

  while (true) {
    LOG_DEBUG("epoll_wait");
    int event_num = epoll_wait(epfd_, &*events.begin(),
                               static_cast<int>(events.size()), -1);
    if (event_num == -1) {
      LOG_FATAL("epoll_wait error");
    }

    if (event_num == static_cast<int>(events.size())) {
      events.resize(events.size() * 2);
    }

    for (int i = 0; i < event_num; ++i) {
      // TODO(pgj): check more situation
      if ((events[i].events & EPOLLIN) != 0U) {
        auto coro_handle =
            std::coroutine_handle<>::from_address(events[i].data.ptr);
        LOG_DEBUG("epoll_await resume in handle");
        coro_handle.resume();

      } else if ((events[i].events & EPOLLOUT) != 0U) {
        auto coro_handle =
            std::coroutine_handle<>::from_address(events[i].data.ptr);
        LOG_DEBUG("epoll_await resume out handle");
        coro_handle.resume();
      }
    }
  }
}

void EventManager::AddRecv(Socket *socket, std::coroutine_handle<> recv_coro) {
  if (!socket->Attached()) {
    // 如果没有attach，先attach
    auto events = EPOLLIN | EPOLLET;
    Attach(socket, recv_coro, events);
  }
  auto new_state = socket->GetIOState() | EPOLLIN;
  socket->SetIOState(new_state);
  UpdateEvent(socket, new_state, recv_coro);
}

void EventManager::DelRecv(Socket *socket) {
  auto new_state = socket->GetIOState() & ~EPOLLIN;
  socket->SetIOState(new_state);
  UpdateEvent(socket, new_state, std::noop_coroutine());
}

void EventManager::AddSend(Socket *socket, std::coroutine_handle<> send_coro) {
  if (!socket->Attached()) {
    auto events = EPOLLOUT | EPOLLET;
    Attach(socket, send_coro, events);
  }

  auto new_state = socket->GetIOState() | EPOLLOUT;
  socket->SetIOState(new_state);
  UpdateEvent(socket, new_state, send_coro);
}

void EventManager::DelSend(Socket *socket) {
  auto new_state = socket->GetIOState() & ~EPOLLOUT;
  socket->SetIOState(new_state);
  UpdateEvent(socket, new_state, std::noop_coroutine());
}

void EventManager::Attach(Socket *socket, std::coroutine_handle<> coro_handle,
                          unsigned int events) {
  struct epoll_event ev {};
  socket->SetIOState(events);
  socket->EventAttach();
  ev.events = events;
  ev.data.ptr = coro_handle.address();
  if (epoll_ctl(epfd_, EPOLL_CTL_ADD, socket->GetFd(), &ev) == -1) {
    LOG_FATAL("epoll_ctl_add: add attach error!\n");
  }
}
void EventManager::Detach(Socket *socket) {
  socket->EventDetach();
  if (epoll_ctl(epfd_, EPOLL_CTL_DEL, socket->GetFd(), nullptr) == -1) {
    LOG_FATAL("epoll_ctl_del: detach error!");
  }
}

void EventManager::UpdateEvent(Socket *socket, unsigned int new_state,
                               std::coroutine_handle<> coro_handle) {
  struct epoll_event ev {};
  ev.events = new_state;
  ev.data.ptr = coro_handle.address();
  if (epoll_ctl(epfd_, EPOLL_CTL_MOD, socket->GetFd(), &ev) == -1) {
    LOG_FATAL("epoll_ctl_mod: error");
  }
}

}  // namespace net
