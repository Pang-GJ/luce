#include "luce/net/tcp/tcp_application.hpp"
#include "luce/common/logger.hpp"
#include "luce/net/tcp/tcp_server.hpp"

namespace net {

coro::Task<> TcpApplication::HandleRequest(TcpConnectionPtr conn,
                                           TcpServer &server) {
  {
    // 这里HandleRequest只被AcceptLoop调用，所以不会存在锁竞争
    // std::lock_guard<std::mutex> lock(mtx_);
    conn_map_[conn->GetSocket()->GetFd()] = conn;
  }
  OnOpen(conn);
  co_await OnRequest(conn, server);
  co_await OnClose(conn);
  {
    // std::lock_guard<std::mutex> lock(mtx_);
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
