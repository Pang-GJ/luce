#pragma once

#include "Channel.h"
#include "Timestamp.h"
#include "noncopyable.h"

#include <unordered_map>
#include <vector>

class Channel;
class EventLoop;

// 多路事件分发器的核心IO复用模块基类
class Poller : noncopyable {
 public:
  using ChannelList = std::vector<Channel *>;

  explicit Poller(EventLoop *loop);
  virtual ~Poller() = default;

  // 给所有IO多路复用提供统一的接口，需要子类重写
  virtual Timestamp poll(int timeoutMs, ChannelList *activeChannels) = 0;
  virtual void updateChannel(Channel *channel) = 0;
  virtual void removeChannel(Channel *channel) = 0;

  // 判断channel是否在当前Poller当中
  virtual bool hasChannel(Channel *channel) const;

  // 获取默认的IO复用的对象
  static Poller *newDefaultPoller(EventLoop *loop);

 protected:
  // <sockfd, Channel>映射map
  using ChannelMap = std::unordered_map<int, Channel *>;
  ChannelMap channels_;

 private:
  EventLoop *ownerLoop_;  // 定义Poller所属的事件循环loop
};
