#include "EventLoop.h"
#include "Poller.h"
#include "EpollPoller.h"

#include <cstdlib>

Poller *Poller::newDefaultPoller(EventLoop *loop) {
  // TODO(pgj): update the nullptr to the real class
  if (::getenv("MUDUO_USE_POLL") != nullptr) {
    return nullptr;  // 生成poll的实例
  }
  // 否则使用epoll
  return new EpollPoller(loop);  // 生成epoll的实例
}
