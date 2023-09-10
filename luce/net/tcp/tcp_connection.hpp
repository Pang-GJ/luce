#pragma once

#include <memory>
#include <vector>
#include "luce/net/socket.hpp"

namespace net {

using IOBuffer = std::vector<char>;

class ReadAwaiter;
class WriteAwaiter;

class TcpConnection : noncopyable,
                      public std::enable_shared_from_this<TcpConnection> {
 public:
  TcpConnection(std::shared_ptr<Socket> sock, EventManager &event_manager);

  ~TcpConnection();

  co::Task<size_t> AsyncRead(IOBuffer *buffer);
  co::Task<size_t> AsyncWrite(const IOBuffer &buffer);

  co::Task<bool> AsyncReadPacket(IOBuffer *buffer);
  co::Task<bool> AsyncWritePacket(const IOBuffer &buffer);

  auto GetEventManager() const -> EventManager & { return event_manager_; }

  auto GetSocket() -> std::shared_ptr<Socket> { return socket_; }

  void Close() {
    if (!is_closed_) {
      socket_->Close();
    }
    is_closed_ = true;
  }

  bool IsClosed() const { return is_closed_; }

 private:
  explicit TcpConnection(EventManager &event_manager)
      : event_manager_(event_manager) {}

  EventManager &event_manager_;
  std::shared_ptr<Socket> socket_;
  std::atomic<bool> is_closed_{false};
};

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
using TcpConnectionWeakPtr = std::weak_ptr<TcpConnection>;

}  // namespace net
