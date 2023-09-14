#pragma once

#include <cerrno>
#include <coroutine>
#include <cstring>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <utility>
#include "luce/codec/abstract_coder.h"
#include "luce/codec/serializer.h"
#include "luce/codec/tinypb_coder.h"
#include "luce/common/logger.h"
#include "luce/io/io_awaiter.h"
#include "luce/net/rpc/invoke_helper.h"
#include "luce/net/rpc/rpc_err_code.h"
#include "luce/net/rpc/rpc_value.h"
#include "luce/net/tcp/tcp_application.h"
#include "luce/net/tcp/tcp_connection.h"

namespace net::rpc {

class RpcServer : public TcpApplication {
 public:
  using HandleFunc =
      std::function<void(codec::Serializer*, codec::Serializer*)>;
  RpcServer() = default;
  ~RpcServer() = default;

  template <typename F>
  void Bind(std::string_view name, F func) {
    handlers_[name.data()] =
        std::bind(&RpcServer::CallProxy<F>, this, func, std::placeholders::_1,
                  std::placeholders::_2);
  }

  template <typename F, typename S>
  void Bind(std::string_view name, F func, S* s) {
    handlers_[name.data()] =
        std::bind(&RpcServer::CallProxy<F, S>, this, func,
                  std::placeholders::_1, std::placeholders::_2);
  }

 private:
  co::Task<> OnRequest(TcpConnectionPtr conn, TcpServer& server) override {
    while (true) {
      IOBuffer buffer;
      auto succ = co_await conn->AsyncReadPacket(&buffer);
      if (!succ) {
        LOG_WARN("RpcServer recv error");
        co_return;
      }

      codec::Serializer serializer(buffer.begin(), buffer.end());

      std::string func_name;
      serializer.deserialize(&func_name);

      codec::Serializer* output_serializer =
          this->CallImpl(func_name, serializer);

      co_await SendResponse(conn, output_serializer);
      delete output_serializer;
    }
    LOG_INFO("RpcServer OnRequest end");
    co_return;
  }

  size_t WritePacket(int fd, const IOBuffer& buffer) {
    size_t total_write_size = buffer.size();
    char head_buffer[net::detail::HEADER_SIZE];
    std::memcpy(head_buffer, reinterpret_cast<char*>(&total_write_size),
                net::detail::HEADER_SIZE);
    auto res = write(fd, head_buffer, net::detail::HEADER_SIZE);
    if (res <= 0) {
      LOG_ERROR("write head error");
      return -1;
    }
    size_t already_write_size = 0;
    while (total_write_size != 0) {
      res = ::write(fd, buffer.data() + already_write_size, total_write_size);
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

  co::Task<> SendResponse(TcpConnectionPtr conn,
                          codec::Serializer* serializer) {
    IOBuffer buffer(serializer->cbegin(), serializer->cend());
    auto succ = co_await conn->AsyncWritePacket(buffer);
    if (!succ) {
      LOG_ERROR("RpcServer SendResponse error");
    }
    co_return;
  }

  codec::Serializer* CallImpl(const std::string& name,
                              codec::Serializer& input_serializer) {
    auto* output_serializer = new codec::Serializer;
    if (!handlers_.contains(name)) {
      output_serializer->serialize(
          RpcResponse<int>::code_type(RPC_ERR_FUNCTION_NOT_FOUND));
      output_serializer->serialize(
          RpcResponse<int>::msg_type("function not bind: " + name));
      LOG_ERROR("function not bind: {}", name);
      return output_serializer;
    }
    auto& func = handlers_[name];
    func(&input_serializer, output_serializer);
    return output_serializer;
  }

  template <typename F>
  void CallProxy(F func, codec::Serializer* input_serializer,
                 codec::Serializer* output_serializer) {
    CallProxy_(func, input_serializer, output_serializer);
  }

  template <typename F, typename S>
  void CallProxy(F func, S* s, codec::Serializer* input_serializer,
                 codec::Serializer* output_serializer) {
    CallProxy_(func, s, input_serializer, output_serializer);
  }

  // 函数指针
  template <typename R, typename... Params>
  void CallProxy_(R (*func)(Params...), codec::Serializer* input_serializer,
                  codec::Serializer* output_serializer) {
    CallProxy_(std::function<R(Params...)>(func), input_serializer,
               output_serializer);
  }

  // 类成员函数指针
  template <typename R, typename C, typename S, typename... Params>
  void CallProxy_(R (C::*func)(Params...), S* s,
                  codec::Serializer* input_serializer,
                  codec::Serializer* output_serializer) {
    using return_type = typename type_xx<R>::type;
    using args_type = std::tuple<typename std::decay<Params>::type...>;
    args_type args;
    input_serializer->deserialize(&args);

    auto ff = [=](Params... params) -> R { return (s->*func)(params...); };
    return_type res = call_helper<R>(ff, args);

    RpcResponse<return_type> value;
    value.err_code = RPC_SUCCECC;
    value.detail_value = res;
    output_serializer->serialize(value);
  }

  // functionnal
  template <typename R, typename... Params>
  void CallProxy_(std::function<R(Params...)> func,
                  codec::Serializer* input_serializer,
                  codec::Serializer* output_serializer) {
    using args_type = std::tuple<typename std::decay<Params>::type...>;
    using return_type = typename type_xx<R>::type;

    args_type args;
    input_serializer->deserialize(&args);

    return_type res = call_helper<R>(func, args);

    RpcResponse<return_type> value;
    value.err_code = RPC_SUCCECC;
    value.detail_value = res;
    output_serializer->serialize(value);
  }

  std::unordered_map<std::string, HandleFunc> handlers_;
};

}  // namespace net::rpc