#pragma once

#include <unistd.h>
#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>

#include "common/CurrentThread.h"
#include "common/Timestamp.h"
#include "common/noncopyable.h"

namespace luce::net {

class Poller;
class Channel;

// 事件循环类
// 主要包含两个大模块 Channel 和 Poller （epoll的抽象）
class EventLoop : noncopyable {
 public:
  using Functor = std::function<void()>;

  EventLoop();
  ~EventLoop();

  // 开启事件循环
  void loop();
  // 退出事件循环
  void quit();

  Timestamp pollRetrunTime() const { return pollRetrunTime_; }

  // 在当前Loop中执行
  void runInLoop(const Functor &cb);
  // 把cb放入到队列中，唤醒Loop所在的线程执行cb
  void queueInLoop(const Functor &cb);

  // 唤醒Loop所在的线程
  void wakeup();

  void updateChannel(Channel *channel);
  void removeChannel(Channel *channel);
  bool hasChannel(Channel *channel);

  // 判断EventLoop对象是否在自己的线程里面
  bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }

 private:
  void handleRead();
  void doPendingFunctors();

  using ChannelList = std::vector<Channel *>;

  // loop的状态控制变量
  std::atomic_bool looping_;  // 原子操作，通过CAS实现
  std::atomic_bool quit_;     // 标识loop循环

  const pid_t threadId_;  // 记录当前loop所在线程的id

  Timestamp pollRetrunTime_;  // Poller返回发生事件的channels的时间点
  std::unique_ptr<Poller> poller_;

  int wakeupFd_;  // 主要作用：当mainloop获取一个新成员的channel
  // 通过轮询算法选择一个subloop，通过wakeupFd_唤醒subloop处理channel
  std::unique_ptr<Channel> wakeupChannel_;

  ChannelList activeChannels_;

  std::atomic_bool callingPendingFunctors_;  // 标识当前loop是否有需要执行的回调操作
  std::vector<Functor> pendingFunctors_;     // 存储loop需要执行的所有回调操作
  std::mutex mutex_;                         // 互斥锁，用来保护 pendingFunctors_的线程安全
};

}  // namespace luce::net