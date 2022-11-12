#include <charconv>
#include <cstring>
#include <utility>

#include "common/logger.hpp"
#include "io/io_awaiter.hpp"
#include "net/event_manager.hpp"
#include "net/tcp_connection.hpp"

namespace net {

TcpConnection::TcpConnection(std::shared_ptr<Socket> sock, EventManager &event_manager)
    : socket_(std::move(sock)), event_manager_(event_manager) {}

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
