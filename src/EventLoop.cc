#include "EventLoop.h"
#include "Channel.h"
#include "CurrentThread.h"
#include "Logger.h"
#include "Poller.h"

#include <fcntl.h>
#include <sys/eventfd.h>
#include <unistd.h>
#include <cerrno>
#include <memory>
#include <mutex>

// 防止一个线程创建多个EventLoop
__thread EventLoop *t_loopInThisThread = nullptr;

// 定义默认的Poller IO复用超时事件：10s
const int kPollTimeMs = 10000;

// 创建wakeupfd，用来notify唤醒subReactor处理新来的channel
int createEventfd() {
  int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (evtfd < 0) {
    LOG_FATAL("eventfd failed: %d\n", errno);
  }

  return evtfd;
}

EventLoop::EventLoop()
    : looping_(false),
      quit_(false),
      threadId_(CurrentThread::tid()),
      poller_(Poller::newDefaultPoller(this)),
      wakeupFd_(createEventfd()),
      wakeupChannel_(new Channel(this, wakeupFd_)),
      callingPendingFunctors_(false) {
  LOG_DEBUG("EventLoop created %p int tread %d \n", this, threadId_);

  if (t_loopInThisThread != nullptr) {
    LOG_FATAL("Anonther EventLoop %p exists in this thread %d \n", this, threadId_);
  } else {
    t_loopInThisThread = this;
  }

  // 设置wakeupFd_的时间类型以及发生事件后的回调操作
  wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
  wakeupChannel_->enableReading();
}

EventLoop::~EventLoop() {
  wakeupChannel_->disableAll();
  wakeupChannel_->remove();
  ::close(wakeupFd_);
  t_loopInThisThread = nullptr;
}

void EventLoop::handleRead() {
  uint64_t one = 1;
  ssize_t n = read(wakeupFd_, &one, sizeof one);
  if (n != sizeof one) {
    LOG_ERROR("EventLoop::handleRead() reads %zd bytes instead of 8\n", n);
  }
}

void EventLoop::loop() {
  looping_ = true;
  quit_ = false;
  LOG_INFO("EventLoop %p start looping \n", this);

  while (!quit_) {
    activeChannels_.clear();
    pollRetrunTime_ = poller_->poll(kPollTimeMs, &activeChannels_);

    for (auto channel : activeChannels_) {
      // Poller监听哪些事件发生了，然后上报给EventLoop，通知channel处理相应的事件
      channel->handleEvent(pollRetrunTime_);
    }

    // 执行当前EventLoop事件循环需要处理的回调操作
    // ???疑惑：这里的回调跟channel的回调分别处理什么？？？
    doPendingFunctors();
  }
}

void EventLoop::quit() {
  quit_ = true;

  // 如果是在其他线程中调用了Loop的quit()，那要把loop唤醒
  // 一个情景是：在一个subLoop中调用了mainLoop的quit，
  if (!isInLoopThread()) {
    wakeup();
  }
}

void EventLoop::runInLoop(const Functor &cb) {
  if (isInLoopThread()) {  // 在当前线程里执行cb
    cb();
  } else {  // 如果不是，那就要唤醒当前loop所在线程执行cb
    queueInLoop(cb);
  }
}

void EventLoop::queueInLoop(const Functor &cb) {
  {
    std::unique_lock<std::mutex> lock(mutex_);
    pendingFunctors_.emplace_back(cb);
  }

  // 唤醒相应的需要执行上面回调函数的loop线程了
  if (!isInLoopThread() || callingPendingFunctors_) { // 当前loop正在执行回调，但是又有了新的回调添加进来
    wakeup();  // 唤醒loop所在线程
  }
}

// 用来唤醒loop所在的线程：只需要向wakeupFd_写一个数据
// 那么wakeupChannel_就会发生读事件，那么loop线程就会被唤醒
void EventLoop::wakeup() {
  uint64_t one = 1;
  ssize_t n = write(wakeupFd_, &one, sizeof one);
  if (n != sizeof one) {
    LOG_ERROR("EventLoop::wakeup() writes %lu bytes instead of 8 \n", n);
  }
}

void EventLoop::updateChannel(Channel *channel) { poller_->updateChannel(channel); }
void EventLoop::removeChannel(Channel *channel) { poller_->removeChannel(channel); }
bool EventLoop::hasChannel(Channel *channel) { return poller_->hasChannel(channel); }

void EventLoop::doPendingFunctors() {
  std::vector<Functor> functors;
  callingPendingFunctors_ = true;

  {
    std::unique_lock<std::mutex> lock(mutex_);
    // 这里的交换有什么好处？
    // 如果没有定义一个局部变量一次性取出所有Functor，而是直接遍历执行的话
    // 会导致其他线程无法向pendingFunctors_里添加（锁的时间很长）
    functors.swap(pendingFunctors_);
  }

  for (const Functor &functor : functors) {
    functor();
  }

  callingPendingFunctors_ = false;
}
