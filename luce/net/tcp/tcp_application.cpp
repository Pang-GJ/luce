#include "luce/net/tcp/tcp_application.h"
#include "luce/common/logger.h"
#include "luce/net/tcp/tcp_server.h"

namespace net {

co::Task<> TcpApplication::HandleRequest(TcpConnectionPtr conn,
                                         TcpServer& server) {
  LOG_DEBUG("handing request");
  co_return co_await OnRequest(conn, server);
}

}  // namespace net
