#include "EventLoopThread.h"
#include "Logger.h"

EventLoopThread::EventLoopThread(const ThreadInitCallback &cb, const std::string &name)
    : loop_(nullptr), exiting_(false), thread_(std::bind(&EventLoopThread::threadFunc, this), name), callback_(cb) {
}

EventLoopThread::~EventLoopThread() {
  exiting_ = true;
  if (loop_ != nullptr) {
    loop_->quit();
    thread_.join();
  }
}

EventLoop *EventLoopThread::startLoop() {
  thread_.start();  // 开启新线程，执行threadFunc

  EventLoop *loop = nullptr;
  {
    std::unique_lock<std::mutex> lock(mutex_);
    while (loop_ == nullptr) {
      cond_.wait(lock);  // 等待threadFunc里notify
    }
    loop = loop_;
  }

  return loop;
}

// threadFunc会单独在一个新线程运行
void EventLoopThread::threadFunc() {
  EventLoop loop;  // 创建一个新的EventLoop，和上面的线程一一对应：
  // one loop per thread,
  if (callback_) {
    callback_(&loop);
  }

  {
    std::unique_lock<std::mutex> lock(mutex_);
    loop_ = &loop;
    cond_.notify_one();
  }

  loop.loop();  // EventLoop loop => Poller.poll

  std::unique_lock<std::mutex> lock(mutex_);
  loop_ = nullptr;
}
