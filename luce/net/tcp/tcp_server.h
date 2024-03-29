#pragma once

#include "luce/co/task.h"
#include "luce/common/noncopyable.h"
#include "luce/common/thread_pool.h"
#include "luce/net/event_manager.h"
#include "luce/net/tcp/tcp_application.h"
#include "luce/timer/timer.h"

#include <atomic>
#include <memory>
#include <vector>

namespace net {

class TcpAcceptor;

class TcpServer : noncopyable {
 public:
  TcpServer(const InetAddress& local_addr, TcpApplication* app,
            size_t thread_num = std::thread::hardware_concurrency());

  ~TcpServer();

  EventManager& GetMainReactor() { return *main_reactor_; }

  EventManager& GetSubReactor() {
    // TODO(pgj): 负载均衡算法
    auto index = cnt_ % sub_reactors_.size();
    cnt_++;
    return *sub_reactors_[index];
  }

  const InetAddress& GetLocalAddr() { return local_addr_; }

  void Start(bool async_start = true);

  void Shutdown();

 private:
  co::Task<void> AcceptLoop();

  std::shared_ptr<EventManager> main_reactor_;
  std::vector<std::shared_ptr<EventManager>>
      sub_reactors_;  // sub reactor有多个, 暂定和线程池数量相同

  std::atomic<size_t> cnt_{0};
  std::atomic<bool> is_shutdown_{false};
  const InetAddress& local_addr_;
  TcpApplication* app_;
  std::unique_ptr<TcpAcceptor> acceptor_;
  std::unique_ptr<ThreadPool> reactor_thread_pool_;
  std::shared_ptr<ThreadPool> work_thread_pool_;
};

}  // namespace net
