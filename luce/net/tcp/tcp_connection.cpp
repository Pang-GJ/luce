#include <charconv>
#include <cstring>
#include <utility>

#include "luce/common/logger.hpp"
#include "luce/io/io_awaiter.hpp"
#include "luce/net/event_manager.hpp"
#include "luce/net/tcp/tcp_connection.hpp"

namespace net {

TcpConnection::TcpConnection(std::shared_ptr<Socket> sock, EventManager &event_manager)
    : event_manager_(event_manager), socket_(std::move(sock)) {}

TcpConnection::~TcpConnection() {
  if (socket_->GetFd() != -1) {
    if (socket_->Attached()) {
      event_manager_.Detach(GetSocket());
    }
  }
}

auto TcpConnection::read(void *buffer, std::size_t len) -> ReadAwaiter {
  return {this, buffer, len};
}

auto TcpConnection::write(void *buffer, std::size_t len) -> WriteAwaiter {
  return {this, buffer, len};
}

}  // namespace net
