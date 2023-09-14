#include <charconv>
#include <cstring>
#include <utility>

#include "luce/common/logger.h"
#include "luce/io/io_awaiter.h"
#include "luce/net/event_manager.h"
#include "luce/net/tcp/tcp_connection.h"

namespace net {

TcpConnection::TcpConnection(std::shared_ptr<Socket> sock,
                             EventManager& event_manager)
    : event_manager_(event_manager), socket_(std::move(sock)) {}

TcpConnection::~TcpConnection() {
  if (socket_->GetFd() != -1) {
    if (socket_->Attached()) {
      event_manager_.Detach(GetSocket());
    }
  }
}

co::Task<size_t> TcpConnection::AsyncRead(IOBuffer* buffer) {
  auto res = co_await ::net::AsyncRead(this, *buffer);
  co_return res;
}

co::Task<size_t> TcpConnection::AsyncWrite(const IOBuffer& buffer) {
  auto res = co_await ::net::AsyncWrite(this, buffer);
  co_return res;
}

co::Task<bool> TcpConnection::AsyncReadPacket(IOBuffer* buffer) {
  auto res = co_await ::net::AsyncReadPacket(this, *buffer);
  co_return res;
}

co::Task<bool> TcpConnection::AsyncWritePacket(const IOBuffer& buffer) {
  auto res = co_await ::net::AsyncWritePacket(this, buffer);
  co_return res;
}

}  // namespace net
