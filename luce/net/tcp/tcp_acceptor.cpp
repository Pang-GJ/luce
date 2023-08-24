#include "luce/net/tcp/tcp_acceptor.hpp"
#include "luce/common/logger.hpp"
#include "luce/io/io_awaiter.hpp"

namespace net {

TcpAcceptor::TcpAcceptor(TcpServer &server, int sock_fd) : server_(server) {
  LOG_INFO("init acceptor");
  socket_ = std::make_shared<Socket>(sock_fd);
  socket_->SetNonblock();
  socket_->SetReuseAddr(true);
  socket_->BindAddress(server.GetLocalAddr());
  socket_->Listen();
  LOG_INFO("init sockfd: {}", socket_->GetFd());
}

auto TcpAcceptor::accept() -> coro::Task<TcpConnectionPtr> {
  int peer_fd = co_await AcceptAwaiter{this};
  if (peer_fd == -1) {
    co_return nullptr;
  }

  auto peer_sock = std::make_shared<Socket>(peer_fd);
  co_return std::make_shared<TcpConnection>(peer_sock, server_.GetMainReactor());
}

}  // namespace net
