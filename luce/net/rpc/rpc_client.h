#pragma once

#include <arpa/inet.h>
#include <asm-generic/errno-base.h>
#include <asm-generic/errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <tuple>
#include "luce/co/task.h"
#include "luce/codec/serializer.h"
#include "luce/common/logger.h"
#include "luce/io/io_awaiter.h"
#include "luce/net/rpc/invoke_helper.h"
#include "luce/net/rpc/rpc_err_code.h"
#include "luce/net/rpc/rpc_value.h"
#include "luce/net/tcp/tcp_application.h"
#include "luce/net/tcp/tcp_connection.h"
namespace net::rpc {

class BlockingRpcClient {
 public:
  BlockingRpcClient(const std::string& server_ip, int server_port) {
    client_fd_ = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    server_addr.sin_addr.s_addr = inet_addr(server_ip.c_str());

    int retry_times = 50;
    while ((retry_times--) != 0) {
      int res = connect(client_fd_, (struct sockaddr*)&server_addr,
                        sizeof(server_addr));
      if (res == 0) {
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
  RpcResponse<R> Call(const std::string& name, Params... params) {
    using args_type = std::tuple<typename std::decay<Params>::type...>;
    args_type args = std::make_tuple(params...);

    codec::Serializer serializer;
    serializer.serialize(name);
    serializer.serialize(args);
    return NetCall<R>(serializer);
  }

  template <typename R>
  RpcResponse<R> Call(const std::string& name) {
    codec::Serializer serializer;
    serializer.serialize(name);
    return NetCall<R>(serializer);
  }

 private:
  template <typename R>
  RpcResponse<R> NetCall(codec::Serializer& serializer) {
    const auto serialized_data = serializer.str();

    IOBuffer request_buffer(serialized_data.cbegin(), serialized_data.cend());
    if (err_code_ != RPC_ERR_RECV_TIMEOUT) {
      auto res = WritePacket(request_buffer);
      if (res != serializer.size()) {
        LOG_FATAL("rpc client send error, errno: {}", errno);
      }
    }

    IOBuffer reply_buffer;
    auto recv_res = ReadPacket(reply_buffer);
    if (recv_res < 0) {
      LOG_ERROR("NetCall get response failed");
    }

    RpcResponse<R> value;
    if (recv_res == 0) {
      err_code_ = RPC_ERR_RECV_TIMEOUT;
      value.err_code = err_code_;
      value.err_msg = "recv timeout";
      return value;
    }
    err_code_ = RPC_SUCCECC;

    codec::Serializer response_serializer(reply_buffer.begin(),
                                          reply_buffer.end());
    response_serializer.deserialize(&value);
    return value;
  }

  size_t ReadPacket(IOBuffer& buffer) {
    char head_buffer[net::detail::HEADER_SIZE];
    size_t head_recv_size = 0;
    while (head_recv_size != net::detail::HEADER_SIZE) {
      auto res = read(client_fd_, head_buffer, net::detail::HEADER_SIZE);
      if (res == 0) {
        LOG_ERROR("recv head error, server closed");
      }
      if (res < 0) {
        if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN) {
          continue;
        }
        LOG_ERROR("recv head error, errno: {}", errno);
        return -1;
      }
      head_recv_size += res;
    }

    uint32_t total_read_size = *reinterpret_cast<uint32_t*>(head_buffer);
    buffer.resize(total_read_size);
    size_t already_read_size = 0;
    while (total_read_size != 0) {
      auto res = ::read(client_fd_, buffer.data() + already_read_size,
                        total_read_size);
      if (res == 0) {
        LOG_WARN("Server cloesd");
        break;
      }
      if (res < 0) {
        if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN) {
          continue;
        }
        break;
      }
      total_read_size -= res;
      already_read_size += res;
    }
    buffer.resize(already_read_size);
    return already_read_size;
  }

  size_t WritePacket(const IOBuffer& buffer) {
    size_t total_write_size = buffer.size();

    char head_buffer[net::detail::HEADER_SIZE];
    std::memcpy(head_buffer, reinterpret_cast<char*>(&total_write_size),
                net::detail::HEADER_SIZE);
    auto res = write(client_fd_, head_buffer, net::detail::HEADER_SIZE);
    if (res <= 0) {
      LOG_ERROR("write head error");
      return -1;
    }
    size_t already_write_size = 0;
    while (total_write_size != 0) {
      res = ::write(client_fd_, buffer.data() + already_write_size,
                    total_write_size);
      if (res == 0) {
        LOG_WARN("Server cloesd");
        break;
      }
      if (res < 0) {
        if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN) {
          continue;
        }
        break;
      }
      already_write_size += res;
      total_write_size -= res;
    }
    return already_write_size;
  }

  int client_fd_{-1};
  int err_code_;
};

}  // namespace net::rpc