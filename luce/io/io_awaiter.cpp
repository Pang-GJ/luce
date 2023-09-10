#include "luce/io/io_awaiter.hpp"
#include <cerrno>

namespace net {
co::Task<size_t> AsyncRead(TcpConnection *conn, IOBuffer &buffer) {
  size_t init_read_size = buffer.size();
  size_t already_read_size = 0;
  bool need_read = true;
  while (need_read) {
    auto res = co_await detail::ReadInnerAwaiter(
        conn, buffer.data() + already_read_size, init_read_size);
    need_read = false;
    if (res == 0) {
      // co_return false;
      break;
    }
    if (res < 0) {
      if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN) {
        need_read = true;
        continue;
      }
      break;
    }
    already_read_size += res;
  }
  buffer.resize(already_read_size);
  co_return already_read_size;
}

co::Task<size_t> AsyncWrite(TcpConnection *conn, const IOBuffer &buffer) {
  size_t total_write_size = buffer.size();
  size_t already_write_size = 0;
  while (total_write_size != 0) {
    auto res = co_await detail::WriteInnerAwaiter(
        conn, buffer.data() + already_write_size, total_write_size);
    if (res == 0) {
      break;
    }
    if (res < 0) {
      if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN) {
        continue;
      }
      break;
    }
    already_write_size += res;
    total_write_size -= res;
  }
  co_return already_write_size;
}

co::Task<bool> AsyncReadPacket(TcpConnection *conn, IOBuffer &buffer) {
  char head_buffer[detail::HEADER_SIZE];
  co_await detail::ReadInnerAwaiter(conn, head_buffer, detail::HEADER_SIZE);
  uint32_t total_read_size = *reinterpret_cast<uint32_t *>(head_buffer);
  buffer.resize(total_read_size);
  uint32_t already_read_size = 0;
  while (total_read_size != 0) {
    auto res = co_await detail::ReadInnerAwaiter(
        conn, buffer.data() + already_read_size, total_read_size);
    if (res == 0) {
      LOG_DEBUG("AsyncReadPacket, client close");
      co_return false;
    }
    if (res < 0) {
      if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN) {
        continue;
      }
      LOG_DEBUG("AsyncReadPacket error, errno: {}", errno);
      co_return false;
    }
    total_read_size -= res;
    already_read_size += res;
  }
  co_return true;
}

co::Task<bool> AsyncWritePacket(TcpConnection *conn, const IOBuffer &buffer) {
  uint32_t total_write_size = buffer.size();
  char head_buffer[detail::HEADER_SIZE];
  std::memcpy(head_buffer, reinterpret_cast<char *>(&total_write_size),
              detail::HEADER_SIZE);
  co_await detail::WriteInnerAwaiter(conn, head_buffer, detail::HEADER_SIZE);
  uint32_t already_write_size = 0;
  while (total_write_size != 0) {
    auto res = co_await detail::WriteInnerAwaiter(
        conn, buffer.data() + already_write_size, total_write_size);
    if (res == 0) {
      LOG_DEBUG("AsyncWritePacket, client close");
      co_return false;
    }
    if (res < 0) {
      if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN) {
        continue;
      }
      LOG_DEBUG("AsyncWritePacket error, errno: {}", errno);
      co_return false;
    }
    total_write_size -= res;
    already_write_size += res;
  }
  co_return true;
}
}  // namespace net