#pragma once

#include <memory>
#include "luce/net/socket.hpp"

namespace net {

class ReadAwaiter;
class WriteAwaiter;

class TcpConnection : noncopyable,
                      public std::enable_shared_from_this<TcpConnection> {
 public:
  TcpConnection(std::shared_ptr<Socket> sock, EventManager &event_manager);

  ~TcpConnection();

  auto read(void *buffer, std::size_t len) -> ReadAwaiter;

  auto write(void *buffer, std::size_t len) -> WriteAwaiter;

  auto GetEventManager() const -> EventManager & { return event_manager_; }

  auto GetSocket() -> std::shared_ptr<Socket> { return socket_; }

  void Close() {
    socket_->Close();
    is_closed_ = true;
  }

  bool IsClosed() const { return is_closed_; }

 private:
  explicit TcpConnection(EventManager &event_manager)
      : event_manager_(event_manager) {}

  EventManager &event_manager_;
  std::shared_ptr<Socket> socket_;
  bool is_closed_{false};
};

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
using TcpConnectionWeakPtr = std::weak_ptr<TcpConnection>;

}  // namespace net
