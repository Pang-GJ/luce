#pragma once

#include <mutex>
#include <unordered_map>

#include "net/tcp_connection.hpp"
#include "coro/task.hpp"

namespace net {

class Socket;
class TcpServer;

class TcpApplication {
  public:
    TcpApplication() = default;
    
    void HandleOpen(TcpConnectionPtr conn);
    void HandleData(TcpConnectionPtr conn, TcpServer &server);
    void HandleClose(TcpConnectionPtr conn);

  protected: 
    virtual coro::Task<void> OnData(TcpConnectionPtr conn, TcpServer &server) = 0;
    virtual coro::Task<void> OnOpen(TcpConnectionPtr conn) = 0;
    virtual coro::Task<void> OnClose(TcpConnectionPtr conn) = 0;

private:
  std::mutex mtx_;
  std::unordered_map<int, TcpConnectionWeakPtr> conn_map_;
};

}  // namespace net
