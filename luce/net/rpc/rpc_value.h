#pragma once

#include <cstdint>
#include "luce/codec/serializer.h"
#include "luce/net/rpc/invoke_helper.h"
namespace net::rpc {

// 暂时将请求大小限制为 64K
constexpr int MAX_VALUE_SIZE = 1024 * 64;

template <typename T>
struct RpcResponse {
  using type = typename type_xx<T>::type;
  using msg_type = std::string;
  using code_type = uint16_t;

  RpcResponse() = default;
  ~RpcResponse() = default;

  T val() const { return detail_value; }

  friend codec::Serializer& operator>>(codec::Serializer& in,
                                       RpcResponse<T>* value) {
    in >> value->err_code >> value->err_msg;
    if (value->err_code == 0) {
      in >> value->detail_value;
    }
    return in;
  }

  friend codec::Serializer& operator<<(codec::Serializer& out,
                                       const RpcResponse<T>& value) {
    out << value.err_code << value.err_msg << value.detail_value;
    return out;
  }

  void serialize(codec::Serializer* serializer) const {
    serializer->serialize(err_code);
    serializer->serialize(err_msg);
    serializer->serialize(detail_value);
  }

  void deserialize(codec::Serializer* serializer) {
    serializer->deserialize(&err_code);
    serializer->deserialize(&err_msg);
    serializer->deserialize(&detail_value);
  }

  code_type err_code{0};
  msg_type err_msg{};
  type detail_value{};
};

}  // namespace net::rpc