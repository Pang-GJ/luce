#pragma once

#include <asm-generic/errno-base.h>
#include <asm-generic/errno.h>
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
#include "luce/common/logger.hpp"
#include "luce/common/stream_buffer.h"
#include "luce/io/io_awaiter.hpp"
#include "luce/net/rpc/invoke_helper.h"
#include "luce/net/rpc/rpc_err_code.h"
#include "luce/net/rpc/rpc_value.h"
#include "luce/net/tcp/tcp_application.hpp"

namespace net::rpc {

class RpcServer : public TcpApplication {
 public:
  using HandleFunc =
      std::function<void(codec::Serializer *, const char *, int)>;
  RpcServer() = default;
  ~RpcServer() = default;

  template <typename F>
  void Bind(std::string_view name, F func) {
    handlers_[name.data()] =
        std::bind(&RpcServer::CallProxy<F>, this, func, std::placeholders::_1,
                  std::placeholders::_2, std::placeholders::_3);
  }

  template <typename F, typename S>
  void Bind(std::string_view name, F func, S *s) {
    handlers_[name.data()] = std::bind(
        &RpcServer::CallProxy<F, S>, this, func, std::placeholders::_1,
        std::placeholders::_2, std::placeholders::_3);
  }

 private:
  co::Task<> OnRequest(TcpConnectionPtr conn, TcpServer &server) override {
    while (true) {
      LOG_INFO("loop start");
      char buffer[MAX_VALUE_SIZE];
      bzero(&buffer, MAX_VALUE_SIZE);
      auto recv_len = co_await conn->read(&buffer, MAX_VALUE_SIZE);
      if (recv_len < 0) {
        if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN) {
          continue;
        }
        break;
      }
      if (recv_len == 0) {
        break;
      }

      StreamBuffer stream_buffer(buffer, recv_len);
      codec::Serializer ds(stream_buffer);

      std::string func_name;
      ds >> func_name;
      // codec::Serializer r;
      codec::Serializer *r =
          this->Call_(func_name, ds.current(), ds.size() - func_name.size());
      LOG_DEBUG("res: {}, size: {}", std::string(r->data()), r->size());

      // LOG_DEBUG("write buffer: {}, len: {}", std::string(write_buffer),
      // strlen(write_buffer));
      if (!Send(conn->GetSocket()->GetFd(), r->data(), r->size())) {
        LOG_ERROR("rpc server send response error");
      }
      // co_await SendResponse(conn, r);
      delete r;
      LOG_INFO("loop end");
    }
    co_return;
  }

  bool Send(int client_fd, const char *buffer, int total_size) {
    ssize_t send_size = 0;
    while (total_size > 0) {
      auto res = ::write(client_fd, buffer + send_size, total_size);
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

  co::Task<> SendResponse(TcpConnectionPtr conn, codec::Serializer *r) {
    auto total_size = r->size();
    char buffer[total_size];
    std::memcpy(buffer, r->data(), total_size);
    ssize_t send_size = 0;
    while (total_size > 0) {
      auto res = co_await conn->write((buffer + send_size), total_size);
      if (res == 0) {
        break;
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
    co_return;
  }

  codec::Serializer *Call_(const std::string &name, const char *data, int len) {
    auto *ds = new codec::Serializer;
    if (!handlers_.contains(name)) {
      (*ds) << RpcValue<int>::code_type(RPC_ERR_FUNCTION_NOT_FOUND);
      LOG_ERROR("function not bind: {}", name);
      (*ds) << RpcValue<int>::msg_type("function not bind: " + name);
      return ds;
    }
    auto &func = handlers_[name];
    func(ds, data, len);
    ds->reset();
    return ds;
  }

  template <typename F>
  void CallProxy(F func, codec::Serializer *pr, const char *data, int len) {
    CallProxy_(func, pr, data, len);
  }

  template <typename F, typename S>
  void CallProxy(F func, S *s, codec::Serializer *pr, const char *data,
                 int len) {
    CallProxy_(func, s, pr, data, len);
  }

  // 函数指针
  template <typename R, typename... Params>
  void CallProxy_(R (*func)(Params...), codec::Serializer *pr, const char *data,
                  int len) {
    CallProxy_(std::function<R(Params...)>(func), pr, data, len);
  }

  // 类成员函数指针
  template <typename R, typename C, typename S, typename... Params>
  void CallProxy_(R (C::*func)(Params...), S *s, codec::Serializer *pr,
                  const char *data, int len) {
    using return_type = typename type_xx<R>::type;
    using args_type = std::tuple<typename std::decay<Params>::type...>;
    codec::Serializer ds(StreamBuffer(data, len));
    constexpr auto N =
        std::tuple_size<typename std::decay<args_type>::type>::value;
    args_type args = ds.get_tuple<args_type>(std::make_index_sequence<N>{});

    auto ff = [=](Params... params) -> R { return (s->*func)(params...); };
    return_type res = call_helper<R>(ff, args);

    RpcValue<R> value;
    value.err_code = RPC_SUCCECC;
    value.detail_value = res;
    (*pr) << value;
  }

  // functionnal
  template <typename R, typename... Params>
  void CallProxy_(std::function<R(Params...)> func, codec::Serializer *pr,
                  const char *data, int len) {
    using args_type = std::tuple<typename std::decay<Params>::type...>;
    using return_type = typename type_xx<R>::type;
    codec::Serializer ds(StreamBuffer(data, len));
    constexpr auto N =
        std::tuple_size<typename std::decay<args_type>::type>::value;
    args_type args = ds.get_tuple<args_type>(std::make_index_sequence<N>{});

    return_type res = call_helper<R>(func, args);

    LOG_INFO("CallProxy_ res: {}", res);
    RpcValue<return_type> value;
    value.err_code = RPC_SUCCECC;
    value.detail_value = res;
    (*pr) << value;
  }

  std::unordered_map<std::string, HandleFunc> handlers_;
};

}  // namespace net::rpc