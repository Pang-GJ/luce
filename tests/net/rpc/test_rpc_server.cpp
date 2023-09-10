#include <string>
#include "luce/common/logger.hpp"
#include "luce/net/inet_address.hpp"
#include "luce/net/rpc/rpc_server.h"
#include "luce/net/rpc_all.h"
#include "luce/net/tcp/tcp_server.hpp"
#include "luce/net/tcp_all.hpp"
#include "spdlog/common.h"
#include "spdlog/spdlog.h"

int add(int a, int b) {
  auto res = a + b;
  LOG_INFO("recv add request: {} + {} = {}", a, b, res);
  return res;
}

int main(int argc, char *argv[]) {
  spdlog::set_level(spdlog::level::debug);
  net::InetAddress addr{12345};
  net::rpc::RpcServer rpc_app;

  rpc_app.Bind("add", add);

  net::TcpServer server(addr, &rpc_app, 8);
  server.Start();
  LOG_INFO("server end");
  return 0;
}
