#pragma once

#include "Poller.h"

#include <sys/epoll.h>
#include <vector>

class EpollPoller : public Poller {
 public:
  explicit EpollPoller(EventLoop *loop);
  ~EpollPoller() override;

  // 重写Poller的接口
  Timestamp poll(int timeoutMs, ChannelList *activeChannels) override;
  void updateChannel(Channel *channel) override;
  void removeChannel(Channel *channel) override;

 private:
  static const int kInitEventListSize = 16;

  // 填写活跃的连接
  void fillActiveChannels(int numEvents, ChannelList *activeChannels);

  // 更新channel
  void update(int operation, Channel *channel);

  using EventList = std::vector<struct epoll_event>;

  int epollfd_;
  EventList events_;
};
