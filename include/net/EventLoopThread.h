#pragma once

#include <condition_variable>
#include <functional>
#include <mutex>
#include <string>

#include "EventLoop.h"
#include "common/Thread.h"
#include "common/noncopyable.h"

namespace luce::net {

class EventLoopThread : noncopyable {
 public:
  using ThreadInitCallback = std::function<void(EventLoop *)>;

  explicit EventLoopThread(const ThreadInitCallback &cb = ThreadInitCallback(),
                           const std::string &name = std::string());
  ~EventLoopThread();

  EventLoop *startLoop();

 private:
  void threadFunc();

  EventLoop *loop_;
  [[maybe_unused]] bool exiting_;
  Thread thread_;
  std::mutex mutex_;
  std::condition_variable cond_;
  ThreadInitCallback callback_;
};

}  // namespace luce::net