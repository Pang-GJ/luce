#include "luce/net/tcp/tcp_server.h"
#include "luce/common/logger.h"
#include "luce/common/singleton.h"
#include "luce/common/thread_pool.h"
#include "luce/net/tcp/tcp_acceptor.h"
#include "luce/timer/timer.h"

namespace net {

TcpServer::TcpServer(const net::InetAddress& local_addr,
                     net::TcpApplication* app, size_t thread_num)
    : local_addr_(local_addr),
      app_(app),
      reactor_thread_pool_(std::make_unique<ThreadPool>(1)),
      work_thread_pool_(std::make_unique<ThreadPool>(thread_num)) {
  LOG_INFO("TcpServer start");
  auto sock_fd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC,
                          IPPROTO_TCP);
  if (sock_fd == -1) {
    LOG_FATAL("create socket error");
  }
  acceptor_ = std::make_unique<TcpAcceptor>(*this, sock_fd);
  main_reactor_ = std::make_shared<EventManager>(nullptr, 2048);

  for (size_t i = 0; i < thread_num / 2 - 1; ++i) {
    sub_reactors_.emplace_back(
        std::make_shared<EventManager>(work_thread_pool_));
  }
  work_thread_pool_->Commit(
      []() { Singleton<timer::TimerManager>::GetInstance()->Tick(); });
}

void TcpServer::Start(bool async_start) {
  if (async_start) {
    reactor_thread_pool_->Commit([&]() {
      AcceptLoop().run();
      LOG_INFO("AcceptLoop().await() done");
    });
  }
  main_reactor_->Start();
}

void TcpServer::Shutdown() {
  is_shutdown_.store(true);
  main_reactor_->Shutdown();
  for (auto& sub_reactor : sub_reactors_) {
    sub_reactor->Shutdown();
  }
  Singleton<timer::TimerManager>::GetInstance()->Shutdown();
  reactor_thread_pool_->Shutdown();
  work_thread_pool_->Shutdown();
  // TODO(pgj): more component to shutdown
}

co::Task<void> TcpServer::AcceptLoop() {
  for (;;) {
    if (is_shutdown_.load()) {
      LOG_DEBUG("TcpServer::AcceptLoop() shutdown");
      break;
    }
    auto conn = co_await acceptor_->accept();
    if (conn != nullptr) {
      work_thread_pool_->Commit(
          [this, conn]() { this->app_->HandleRequest(conn, *this).run(); });
    }
  }
}
TcpServer::~TcpServer() { LOG_INFO("TcpServer end"); }

}  // namespace net
