#include "net/tcp_application.hpp"
#include "net/tcp_connection.hpp"
#include "net/tcp_server.hpp"

namespace net {

void TcpApplication::HandleOpen(TcpConnectionPtr conn) {
  {
    std::lock_guard<std::mutex> lock(mtx_);
    conn_map_[conn->GetSocket()->GetFd()] = conn;
  }
  OnOpen(std::move(conn));
}

void TcpApplication::HandleClose(TcpConnectionPtr conn) {
  {
    std::lock_guard<std::mutex> lock(mtx_);
    conn_map_.erase(conn->GetSocket()->GetFd());
  }
  OnClose(std::move(conn));
}

void TcpApplication::HandleData(TcpConnectionPtr conn, TcpServer &server) {
  OnData(std::move(conn), server);
}

}  // namespace net
