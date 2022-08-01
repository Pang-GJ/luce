#include "EventLoopThreadPool.h"
#include "EventLoopThread.h"

EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseLoop, const std::string &name)
    : baseLoop_(baseLoop), started_(false), numThreads_(0), next_(0) {
  name_ = name;
}

void EventLoopThreadPool::start(const ThreadInitCallback &cb) {
  started_ = true;

  for (int i = 0; i < numThreads_; ++i) {
    char buf[name_.size() + 32];
    snprintf(buf, sizeof buf, "%s%d", name_.c_str(), i);
    EventLoopThread *t = new EventLoopThread(cb, buf);
    threads_.emplace_back(std::unique_ptr<EventLoopThread>(t));
    loops_.emplace_back(t->startLoop());  // 底层创建线程，绑定一个新的EventLoop
  }

  if (numThreads_ == 0 && cb) {
    // 如果numThreads_ == 0,那么整个服务端只有一个线程
    cb(baseLoop_);
  }
}

EventLoop *EventLoopThreadPool::getNextLoop() {
  EventLoop *loop = baseLoop_;

  if (!loops_.empty()) {
    // round-robin
    loop = loops_[next_];
    ++next_;
    if (next_ >= loops_.size()) {
      next_ = 0;
    }
  }

  return loop;
}

std::vector<EventLoop *> EventLoopThreadPool::getAllLoops() {
  if (loops_.empty()) {
    return {baseLoop_};
  }
  return loops_;
}
