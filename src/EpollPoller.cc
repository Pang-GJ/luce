#include "EpollPoller.h"
#include "Logger.h"

#include <sys/epoll.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>

// channel未添加到Poller中
const int kNew = -1;  // channel中的成员index_ = -1
// channel已添加到Poller中
const int kAdded = 1;
// channel从Poller中删除
const int kDeleted = 2;

EpollPoller::EpollPoller(EventLoop *loop)
    : Poller(loop), epollfd_(epoll_create1(EPOLL_CLOEXEC)), events_(kInitEventListSize) {
  if (epollfd_ < 0) {
    LOG_FATAL("epoll_create error: %d\n", errno);
  }
}

EpollPoller::~EpollPoller() { ::close(epollfd_); }

// 重写Poller的接口, 封装了epoll_wait
// 这里使用的是epoll的LT模式
Timestamp EpollPoller::poll(int timeoutMs, ChannelList *activeChannels) {
  // 实际上利用LOG_DEBUG输出日志更合理
  LOG_INFO("func=%s => fd total count: %lu\n", __FUNCTION__, channels_.size());

  // epoll_wait要求传入event数组的地址，但events_是vector，怎么办？
  // 一个新知识：events_.begin() 是首元素的迭代器，*events_.begin()就是首元素
  // 然后再对*events_.begin()取地址就是首元素的地址，就是底层数组的首地址
  int numEvents = ::epoll_wait(epollfd_, &*events_.begin(), static_cast<int>(events_.size()), timeoutMs);
  int savedErrno = errno;
  Timestamp now(Timestamp::now());

  if (numEvents > 0) {
    LOG_INFO("%d events happened\n", numEvents);
    fillActiveChannels(numEvents, activeChannels);
    // 判断是否需要对events_扩容
    if (numEvents == static_cast<int>(events_.size())) {
      events_.resize(events_.size() * 2);
    }

  } else if (numEvents == 0) {
    LOG_DEBUG("%s timeout! \n", __FUNCTION__);
  } else {
    // error happens,
    // EINTR表示系统调用被中断信号打断,如果只是中断信号，可以继续
    if (savedErrno != EINTR) {
      errno = savedErrno;
      LOG_ERROR("EpollPoller::poll() error!\n");
    }
  }

  return now;
}

void EpollPoller::updateChannel(Channel *channel) {
  const int index = channel->index();
  LOG_INFO("func=%s => fd=%d, events=%d, index=%d\n", __FUNCTION__, channel->fd(), channel->events(), index);

  if (index == kNew || index == kDeleted) {
    if (index == kNew) {
      int fd = channel->fd();
      channels_[fd] = channel;
    }

    channel->set_index(kAdded);
    update(EPOLL_CTL_ADD, channel);

  } else {
    // update existing one with EPOLL_CTL_MOD/DEL
    if (channel->isNoneEvent()) {
      update(EPOLL_CTL_DEL, channel);
      channel->set_index(kDeleted);
    } else {
      update(EPOLL_CTL_MOD, channel);
    }
  }
}

void EpollPoller::removeChannel(Channel *channel) {
  int fd = channel->fd();
  int index = channel->index();

  LOG_INFO("func=%s => fd=%d\n", __FUNCTION__, channel->fd());
  channels_.erase(fd);

  if (index == kAdded) {
    update(EPOLL_CTL_DEL, channel);
  }
  channel->set_index(kNew);
}

// 填写活跃的连接
void EpollPoller::fillActiveChannels(int numEvents, ChannelList *activeChannels) {
  for (int i = 0; i < numEvents; ++i) {
    Channel *channel = static_cast<Channel *>(events_[i].data.ptr);
    channel->set_revents(static_cast<int>(events_[i].events));

    // activeChannels->push_back(channel);
    activeChannels->emplace_back(channel);
  }
}

// 更新channel 封装了epoll_ctl
void EpollPoller::update(int operation, Channel *channel) {
  struct epoll_event event;
  memset(&event, 0, sizeof event);
  int fd = channel->fd();
  event.events = channel->events();
  event.data.ptr = channel;
  event.data.fd = fd;

  if (::epoll_ctl(epollfd_, operation, fd, &event) < 0) {
    if (operation == EPOLL_CTL_DEL) {
      LOG_ERROR("epoll_ctl del error, erron: %d\n", errno);
    } else {
      LOG_FATAL("epoll_ctl add/mod error, erron: %d\n", errno);
    }
  }
}
