#include "luce/net/tcp/tcp_server.hpp"
#include "luce/net/tcp/tcp_acceptor.hpp"

namespace net {

TcpServer::TcpServer(const net::InetAddress &local_addr,
                     ThreadPool &thread_pool, net::TcpApplication *app)
    : local_addr_(local_addr), thread_pool_(thread_pool), app_(app) {
  LOG_INFO("TcpServer start");
  auto sock_fd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC,
                          IPPROTO_TCP);
  if (sock_fd == -1) {
    LOG_FATAL("create socket error");
  }
  acceptor_ = std::make_unique<TcpAcceptor>(*this, sock_fd);

  main_reactor_ = std::make_shared<EventManager>(128);
  //  thread_pool_.Commit([this]() { main_reactor_->Start(); });

  auto sz = thread_pool_.Size();
  for (size_t i = 0; i < sz / 2; ++i) {
    sub_reactors_.emplace_back(std::make_shared<EventManager>());
    thread_pool_.Commit([this, i]() { this->sub_reactors_[i]->Start(); });
  }
}

void TcpServer::Start(bool async_start) {
  if (async_start) {
    thread_pool_.Commit([&]() { AcceptLoop(); });
  }
  main_reactor_->Start();
}

void TcpServer::Shutdown() {
  is_shutdown_.store(true);
  main_reactor_->Shutdown();
  for (auto &sub_reactor : sub_reactors_) {
    sub_reactor->Shutdown();
  }
  thread_pool_.Shutdown();
  // TODO(pgj): more component to shutdown
}

coro::Task<void> TcpServer::AcceptLoop() {
  for (;;) {
    if (is_shutdown_.load()) {
      break;
    }
    auto conn = co_await acceptor_->accept();
    if (conn != nullptr) {
      thread_pool_.Commit(
          [this, conn]() { this->app_->HandleRequest(conn, *this); });
    }
  }
}
TcpServer::~TcpServer() { LOG_INFO("TcpServer end"); }

}  // namespace net