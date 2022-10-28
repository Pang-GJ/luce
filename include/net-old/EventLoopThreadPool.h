#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "common/noncopyable.h"

namespace luce::net {

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : noncopyable {
 public:
  using ThreadInitCallback = std::function<void(EventLoop *)>;

  EventLoopThreadPool(EventLoop *baseLoop, const std::string &name);
  ~EventLoopThreadPool() = default;  // dont' delete loop, it's stack variable

  void setThreadNum(int numThreads) { numThreads_ = numThreads; }

  void start(const ThreadInitCallback &cb = ThreadInitCallback());

  // 如果工作在多线程模式中，baseLoop_默认以轮询方式分配channel给subloop
  EventLoop *getNextLoop();

  std::vector<EventLoop *> getAllLoops();

  bool started() const { return started_; }

  const std::string &name() const { return name_; }

 private:
  EventLoop *baseLoop_;
  std::string name_;
  bool started_;
  int numThreads_;
  int next_;
  std::vector<std::unique_ptr<EventLoopThread>> threads_;
  std::vector<EventLoop *> loops_;
};

}  // namespace luce::net