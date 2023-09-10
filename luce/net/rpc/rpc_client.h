#pragma once

#include <arpa/inet.h>
#include <asm-generic/errno-base.h>
#include <asm-generic/errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cerrno>
#include <tuple>
#include "luce/co/task.hpp"
#include "luce/codec/serializer.h"
#include "luce/common/logger.hpp"
#include "luce/io/io_awaiter.hpp"
#include "luce/net/rpc/invoke_helper.h"
#include "luce/net/rpc/rpc_err_code.h"
#include "luce/net/rpc/rpc_value.h"
#include "luce/net/tcp/tcp_application.hpp"
namespace net::rpc {

class BlockingRpcClient {
 public:
  BlockingRpcClient(const std::string &server_ip, int server_port) {
    client_fd_ = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    server_addr.sin_addr.s_addr = inet_addr(server_ip.c_str());

    int retry_times = 50;
    while ((retry_times--) != 0) {
      int res = connect(client_fd_, (struct sockaddr *)&server_addr,
                        sizeof(server_addr));
      if (res == 0) {
        LOG_DEBUG("blocking rpc client connect success.");
        break;
      }
      if (res < 0) {
        if (errno == EINPROGRESS) {
          continue;
        }
        LOG_FATAL("blocking rpc client connect failed, errno: {}", errno);
      }
    }
  }

  ~BlockingRpcClient() {
    if (client_fd_ != -1) {
      ::close(client_fd_);
    }
  }

  template <typename R, typename... Params>
  RpcValue<R> Call(const std::string &name, Params... params) {
    using args_type = std::tuple<typename std::decay<Params>::type...>;
    args_type args = std::make_tuple(params...);

    codec::Serializer ds;
    ds << name;
    package_params(ds, args);
    return NetCall<R>(ds);
  }

  template <typename R>
  RpcValue<R> Call(const std::string &name) {
    codec::Serializer ds;
    ds << name;
    return NetCall<R>(ds);
  }

 private:
  template <typename R>
  RpcValue<R> NetCall(codec::Serializer &ds) {
    char request_buffer[ds.size() + 1];
    std::memcpy(request_buffer, ds.data(), ds.size());
    if (err_code_ != RPC_ERR_RECV_TIMEOUT) {
      if (!Send(request_buffer, sizeof(request_buffer))) {
        LOG_FATAL("rpc client send error, errno: {}", errno);
      }
    }

    char reply_buffer[MAX_VALUE_SIZE];
    auto recv_len = Recv(reply_buffer);
    RpcValue<R> value;
    if (recv_len == 0) {
      err_code_ = RPC_ERR_RECV_TIMEOUT;
      value.err_code = err_code_;
      value.err_msg = "recv timeout";
      return value;
    }
    err_code_ = RPC_SUCCECC;
    ds.clear();
    ds.write_raw_data(reply_buffer, recv_len);
    ds.reset();

    ds >> value;
    return value;
  }

  bool Send(char *buffer, int total_size) {
    ssize_t send_size = 0;
    while (total_size > 0) {
      auto res = send(client_fd_, reinterpret_cast<void *>(buffer + send_size),
                      total_size, 0);
      if (res == 0) {
        return false;
      }
      if (res < 0) {
        if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN) {
          continue;
        }
        break;
      }
      total_size -= res;
      send_size += res;
    }
    return true;
  }

  int Recv(char *buffer) {
    int recv_size = 0;
    int res = 0;
    while (true) {
      res = recv(client_fd_, buffer + recv_size, 512, 0);
      if (res == 0) {
        break;
      }
      if (res < 0) {
        if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN) {
          LOG_DEBUG("recv continue");
          continue;
        }
        break;
      }
      recv_size += res;
    }
    LOG_INFO("client recv size: {}", recv_size);
    return recv_size;
  }

  int client_fd_{-1};
  int err_code_;
};

/*/
class RpcClient : public TcpApplication {
 public:
  RpcClient() = default;
  ~RpcClient() = default;

  template <typename R, typename... Params>
  co::Task<RpcValue<R>> Call(const std::string &name, Params... params) {
    using args_type = std::tuple<typename std::decay<Params>::type...>;
    args_type args = std::make_tuple(params...);

    codec::Serializer ds;
    ds << name;
    package_params(ds, args);
    auto res = co_await NetCall<R>(ds);
    co_return res;
  }

  template <typename R>
  co::Task<RpcValue<R>> Call(const std::string &name) {
    codec::Serializer ds;
    ds << name;
    auto res = co_await NetCall<R>(ds);
    co_return res;
  }

 private:
  co::Task<> OnRequest(TcpConnectionPtr conn, TcpServer &server) override {
    while (true) {
    }
    co_return;
  }

  co::Task<> SendResponse(TcpConnectionPtr conn, char *buffer, int total_size) {
    ssize_t send_size = 0;
    while (total_size > 0) {
      auto res = co_await conn->write(
          reinterpret_cast<void *>(buffer + send_size), total_size);
      if (res <= 0) {
        break;
      }
      total_size -= res;
      send_size += res;
    }
  }
};
*/

}  // namespace net::rpc