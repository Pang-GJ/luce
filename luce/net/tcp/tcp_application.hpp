#pragma once

#include <mutex>
#include <unordered_map>

#include "luce/coro/task.hpp"
#include "luce/net/tcp/tcp_connection.hpp"

namespace net {

class Socket;
class TcpServer;

class TcpApplication {
 public:
  TcpApplication() = default;

  coro::Task<> HandleRequest(TcpConnectionPtr conn, TcpServer &server);
  //    void HandleData(TcpConnectionPtr conn, TcpServer &server);
  //    void HandleClose(TcpConnectionPtr conn);

 protected:
  virtual coro::Task<> OnRequest(TcpConnectionPtr conn,
                                     TcpServer &server) = 0;
  virtual coro::Task<> OnOpen(TcpConnectionPtr conn) = 0;
  virtual coro::Task<> OnClose(TcpConnectionPtr conn) = 0;

 private:
  std::mutex mtx_;
  std::unordered_map<int, TcpConnectionWeakPtr> conn_map_;
};

}  // namespace net
