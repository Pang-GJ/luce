#pragma once

#include <functional>
#include <memory>

#include "Timestamp.h"

class Buffer;
class TcpConnction;

using TcpConnctionPtr = std::shared_ptr<TcpConnction>;

using ConnectionCallback = std::function<void(const TcpConnctionPtr &)>;
using CloseCallback = std::function<void(const TcpConnctionPtr &)>;
using WriteCompleteCallback = std::function<void(const TcpConnctionPtr &)>;

using MessageCallback = std::function<void(const TcpConnctionPtr &, Buffer *, Timestamp)>;
