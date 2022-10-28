#pragma once

#include "common/Timestamp.h"
#include "common/noncopyable.h"

#include <functional>
#include <memory>

namespace luce::net {

class EventLoop;

// 理清楚EventLoop、Channel、Poller之间的关系
// Channel理解为通道，封装了fd和监听的event
// 还绑定了poller返回的具体事件
class Channel : noncopyable {
 public:
  using EventCallback = std::function<void()>;
  using ReadEventCallback = std::function<void(Timestamp)>;

  Channel(EventLoop *loop, int fd);
  ~Channel() = default;

  // fd得到Poller通知以后处理事件（调用相应的回调方法）
  void handleEvent(Timestamp receiveTime);

  // 设置回调函数对象
  void setReadCallback(ReadEventCallback cb) { readCallback_ = std::move(cb); }
  void setWriteCallback(EventCallback cb) { writeCallback_ = std::move(cb); }
  void setCloseCallback(EventCallback cb) { closeCallback_ = std::move(cb); }
  void setErrorCallback(EventCallback cb) { errorCallback_ = std::move(cb); }

  // 当Channel被手动remove掉，防止channel还在执行回调操作
  void tie(const std::shared_ptr<void> &);

  int fd() const { return fd_; }
  int events() const { return events_; }
  int revents() const { return revents_; }

  // 返回fd当前的事件状态
  bool isNoneEvent() const { return events_ == kNoneEvent; }
  bool isWriting() const { return (events_ & kWriteEvent) != 0; }
  bool isReading() const { return (events_ & kReadEvent) != 0; }

  // 设置fd相应的事件 （epoll_ctl)
  void enableReading() {
    events_ |= kReadEvent;
    update();
  }
  void disableReading() {
    events_ &= ~kReadEvent;
    update();
  }
  void enableWriting() {
    events_ |= kWriteEvent;
    update();
  }
  void disableWriting() {
    events_ &= ~kWriteEvent;
    update();
  }
  void disableAll() {
    events_ = kNoneEvent;
    update();
  }

  // for Poller
  int index() const { return index_; }
  void set_index(int idx) { index_ = idx; }
  void set_revents(int revents) { revents_ = revents; }

  // 当前Channel属于哪个EventLoop
  EventLoop *ownerLoop() const { return loop_; }

  void remove();

 private:
  void update();
  void handleEventWithGuard(Timestamp receiveTime);

  static const int kNoneEvent;
  static const int kReadEvent;
  static const int kWriteEvent;

  EventLoop *loop_;  // 事件循环
  const int fd_;     // fd, Poller监听的对象
  int events_;       // 注册fd监听的事件
  int revents_;      // poller返回的具体发生的事件
  int index_;

  std::weak_ptr<void> tie_;
  bool tied_;

  // 因为Channel通道里面能够获知fd最终发生的事件revents，所以它负责调用具体的回调
  ReadEventCallback readCallback_;
  EventCallback writeCallback_;
  EventCallback closeCallback_;
  EventCallback errorCallback_;
};

}  // namespace luce::net