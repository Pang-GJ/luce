#include "net/Poller.h"
#include "net/EventLoop.h"

namespace luce::net {

Poller::Poller(EventLoop *loop) : ownerLoop_(loop) {}

bool Poller::hasChannel(Channel *channel) const {
  auto it = channels_.find(channel->fd());
  return it != channels_.end() && it->second == channel;
}

} // namespace luce::net