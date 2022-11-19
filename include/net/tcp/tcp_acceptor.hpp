#pragma once

#include "net/event_manager.hpp"
#include "net/tcp/tcp_connection.hpp"
#include "net/tcp/tcp_server.hpp"

namespace net {

class TcpAcceptor {
 public:
  explicit TcpAcceptor(TcpServer &server, int sock_fd);

  ~TcpAcceptor() {
    LOG_INFO("delete acceptor");
  }

  auto accept() -> coro::Task<TcpConnectionPtr>;

  auto GetEventManager() const -> EventManager & {
    return server_.GetMainReactor();
  }

  auto GetSocket() -> std::shared_ptr<Socket> { return socket_; }

 private:
  TcpServer &server_;
  std::shared_ptr<Socket> socket_;
};

};  // namespace net
