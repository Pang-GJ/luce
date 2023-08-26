#pragma once

#include <memory>
#include <string>
namespace codec {

class AbstractProtocol : std::enable_shared_from_this<AbstractProtocol> {
 public:
  using Ptr = std::shared_ptr<AbstractProtocol>;

  virtual ~AbstractProtocol() = default;

  // 请求号，标识唯一一个请求或者响应
  std::string msg_id;  // NOLINT
};

}  // namespace codec