#include "common/logger.hpp"
#include "coro/task.hpp"
#include "net/socket.hpp"
#include "net/io_awaiter.hpp"

coro::Task<bool> inside_loop(net::Socket &socket) {
  char buffer[1024] = {0};
  ssize_t recv_len = co_await socket.recv(buffer, sizeof buffer);
  ssize_t send_len = 0;
  while (send_len < recv_len) {
    ssize_t res = co_await socket.send(buffer, sizeof buffer);
    if (res <= 0) {
      co_return false;
    }
    send_len += res;
  }
  LOG_INFO("Done send\n");
  if (recv_len <= 0) {
    co_return false;
  }
  printf("buffer: %s\n", buffer);
  co_return true;
}

coro::Task<> echo_socket(std::shared_ptr<net::Socket> socket) {
  while (true) {
    LOG_INFO("socket %d BEGIN!\n", socket->GetFd());
    bool b = co_await inside_loop(*socket);
    if (!b) {
      break;
    }
    LOG_INFO("socket %d END!\n", socket->GetFd());
  }
}

coro::Task<> accept(net::Socket &listen_sock) {
  while (true) {
    auto socket = co_await listen_sock.accept();
    if (socket->GetFd() != -1) {
      auto t = echo_socket(socket);
    }
    // t.resume();
  }
}

int main(int argc, char *argv[]) {
  // net::IOContext io_context;
  net::EventManager event_manager;
  net::Socket listen_sock{"127.0.0.1", 10009, event_manager};
  auto t = accept(listen_sock);
  // t.resume();

  // io_context.Run();
  event_manager.Start();
}
