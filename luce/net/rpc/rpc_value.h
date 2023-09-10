#pragma once

#include <cstdint>
#include "luce/codec/serializer.h"
#include "luce/net/rpc/invoke_helper.h"
namespace net::rpc {

// 暂时将请求大小限制为 64K
constexpr int MAX_VALUE_SIZE = 1024 * 64;

template <typename T>
struct RpcValue {
  using type = typename type_xx<T>::type;
  using msg_type = std::string;
  using code_type = uint16_t;

  RpcValue() = default;
  ~RpcValue() = default;

  T val() const { return detail_value; }

  friend codec::Serializer &operator>>(codec::Serializer &in,
                                       RpcValue<T> *value) {
    in >> value->err_code >> value->err_msg;
    if (value->err_code == 0) {
      in >> value->detail_value;
    }
    return in;
  }

  friend codec::Serializer &operator<<(codec::Serializer &out,
                                       const RpcValue<T> &value) {
    out << value.err_code << value.err_msg << value.detail_value;
    return out;
  }

  code_type err_code{0};
  msg_type err_msg{};
  type detail_value{};
};

}  // namespace net::rpc