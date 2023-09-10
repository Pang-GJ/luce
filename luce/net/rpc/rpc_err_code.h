#pragma once

namespace net::rpc {

enum RpcErrCode {
  RPC_SUCCECC = 0,
  RPC_ERR_FUNCTION_NOT_FOUND,
  RPC_ERR_RECV_TIMEOUT
};

}