#include <string>
#include "luce/common/logger.hpp"
#include "luce/net/rpc/rpc_client.h"
#include "luce/net/rpc_all.h"

int main(int argc, char *argv[]) {
  spdlog::set_level(spdlog::level::debug);
  net::rpc::BlockingRpcClient client("127.0.0.1", 12345);
  int res = client.Call<int>("add", 2, 3).val();
  LOG_INFO("call add response: {}", res);
  return 0;
}