#include "net/EpollPoller.h"
#include "net/EventLoop.h"
#include "net/Poller.h"

#include <cstdlib>

namespace luce::net {

Poller *Poller::newDefaultPoller(EventLoop *loop) {
  // TODO(pgj): update the nullptr to the real class
  if (::getenv("MUDUO_USE_POLL") != nullptr) {
    return nullptr;  // 生成poll的实例
  }
  // 否则使用epoll
  return new EpollPoller(loop);  // 生成epoll的实例
}

}