#include "net/tcp_application.hpp"
#include "common/logger.hpp"
#include "net/tcp_connection.hpp"
#include "net/tcp_server.hpp"

namespace net {

coro::Task<> TcpApplication::HandleRequest(TcpConnectionPtr conn,
                                           TcpServer &server) {
  {
    std::lock_guard<std::mutex> lock(mtx_);
    conn_map_[conn->GetSocket()->GetFd()] = conn;
  }
  OnOpen(conn);
  co_await OnRequest(conn, server);
  co_await OnClose(conn);
  {
    std::lock_guard<std::mutex> lock(mtx_);
    conn_map_.erase(conn->GetSocket()->GetFd());
  }
}

// void TcpApplication::HandleClose(TcpConnectionPtr conn) {
//   {
//     std::lock_guard<std::mutex> lock(mtx_);
//     conn_map_.erase(conn->GetSocket()->GetFd());
//   }
//   OnClose(std::move(conn));
// }
//
// void TcpApplication::HandleData(TcpConnectionPtr conn,
//                                             TcpServer &server) {
//   OnRequest(std::move(conn), server);
// }

}  // namespace net
