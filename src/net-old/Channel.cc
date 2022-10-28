#include "net/Channel.h"
#include "common/logger.hpp"
#include "net/EventLoop.h"

#include <sys/epoll.h>

namespace luce::net {

// 跟muduo不同的是这里只实现了Epoll，muduo还实现了poll
// 但我后期可能考虑重新设计这个网络库，可能考虑最新的io_uring
const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::kWriteEvent = EPOLLOUT;

Channel::Channel(EventLoop *loop, int fd) : loop_(loop), fd_(fd), events_(0), revents_(0), index_(-1), tied_(false) {}

// Channel::~Channel() {}

void Channel::tie(const std::shared_ptr<void> &obj) {
  tie_ = obj;
  tied_ = true;
}

// 当改变Channel所表示fd的events事件后，
// update负责在Poller里面更改fd相应的事件epoll_ctl
void Channel::update() {
  // 通过Channel所属的EventLoop调用Poller的相应方法
  // 然后在Poller里注册fd的events事件
  loop_->updateChannel(this);
}

// 在Channel所属的EventLoop中将当前的Channel删除掉
void Channel::remove() { loop_->removeChannel(this); }

void Channel::handleEvent(Timestamp receiveTime) {
  if (tied_) {
    std::shared_ptr<void> guard = tie_.lock();
    if (guard) {
      handleEventWithGuard(receiveTime);
    }
  } else {
    handleEventWithGuard(receiveTime);
  }
}

// 根据Poller通知的具体事件，由Channel负责调用具体的回调操作
void Channel::handleEventWithGuard(Timestamp receiveTime) {
  LOG_INFO("channel handleEvent revents: %d", revents_);

  if (((revents_ & EPOLLHUP) != 0) && ((revents_ & EPOLLIN) == 0)) {
    if (closeCallback_) {
      closeCallback_();
    }
  }

  if ((revents_ & EPOLLERR) != 0) {
    if (errorCallback_) {
      errorCallback_();
    }
  }

  if ((revents_ & (EPOLLIN | EPOLLPRI)) != 0) {
    if (readCallback_) {
      readCallback_(receiveTime);
    }
  }

  if ((revents_ & EPOLLOUT) != 0) {
    if (writeCallback_) {
      writeCallback_();
    }
  }
}

}  // namespace luce::net