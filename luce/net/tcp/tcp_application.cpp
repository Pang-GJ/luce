#include "luce/net/tcp/tcp_application.hpp"
#include "luce/common/logger.hpp"
#include "luce/net/tcp/tcp_server.hpp"

namespace net {

coro::Task<> TcpApplication::HandleRequest(TcpConnectionPtr conn,
                                           TcpServer &server) {
  co_await OnRequest(conn, server);
}

}  // namespace net
