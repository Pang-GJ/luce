#pragma once

#include <functional>
#include <memory>

#include "common/Timestamp.h"
#include "common/logger.hpp"

namespace luce::net {

class Buffer;
class TcpConnection;

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

using ConnectionCallback = std::function<void(const TcpConnectionPtr &)>;
using CloseCallback = std::function<void(const TcpConnectionPtr &)>;
using WriteCompleteCallback = std::function<void(const TcpConnectionPtr &)>;

using MessageCallback = std::function<void(const TcpConnectionPtr &, Buffer *, Timestamp)>;

using HighWaterMarkCallback = std::function<void(const TcpConnectionPtr &, size_t)>;

void defaultConnectionCallback(const TcpConnectionPtr &conn);

void defaultMessageCallback(const TcpConnectionPtr &, Buffer *buf, Timestamp);

}  // namespace luce::net