#pragma once

#include "common/noncopyable.h"
#include "net/event_manager.hpp"

#include <vector>

namespace net {

class TcpServer : noncopyable {
 public:
  EventManager &GetMainReactor() { return *main_reactor_; }

  EventManager &GetSubReactor() {
    // TODO(pgj): 负载均衡算法
    auto index = cnt_ % sub_reactors_.size();
    cnt_++;
    return *sub_reactors_[index];
  }

  const InetAddress &GetLocalAddr() { return local_addr_; }

 private:
  EventManager *main_reactor_;
  std::vector<EventManager *> sub_reactors_;  // sub reactor有多个, 暂定和线程池数量相同
  size_t cnt_ {0};
  const InetAddress local_addr_;
};

}  // namespace net
