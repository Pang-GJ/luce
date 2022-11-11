#include "net/tcp_acceptor.hpp"
#include "io/io_awaiter.hpp"

namespace net {

TcpAcceptor::TcpAcceptor(TcpServer &server, int sock_fd) : server_(server) {
  socket_ = std::make_shared<Socket>(sock_fd);
  auto local_addr = server_.GetLocalAddr();
  socket_->SetNonblock();
  socket_->SetReusePort(true);
  socket_->BindAddress(local_addr);
  socket_->Listen();
}

auto TcpAcceptor::accept() -> coro::Task<TcpConnectionPtr> {
  int peer_fd = co_await AcceptAwaiter{this};

  auto peer_sock = std::make_shared<Socket>(peer_fd);
  co_return std::make_shared<TcpConnection>(peer_sock, server_.GetSubReactor());
}

}  // namespace net
