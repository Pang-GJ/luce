#include "common/logger.hpp"
#include "coro/task.hpp"
#include "net/io_awaiter.hpp"
#include "net/socket.hpp"

#include <csignal>  // for ignore SIGPIPE
#include <thread>

coro::Task<bool> do_io(net::Socket &socket) {
  char buffer[512] = {0};
  ssize_t recv_len = co_await socket.read(buffer, sizeof buffer);
  ssize_t send_len = 0;
  while (send_len < recv_len) {
    ssize_t res = co_await socket.write(buffer, sizeof buffer);
    if (res <= 0) {
      co_return false;
    }
    send_len += res;
  }
  LOG_DEBUG("Done send\n");
  if (recv_len <= 0) {
    co_return false;
  }
  LOG_DEBUG("buffer: %s\n", buffer);
  co_return true;
}

coro::Task<> do_io_loop(std::shared_ptr<net::Socket> socket) {
  while (true) {
    LOG_DEBUG("socket %d BEGIN!\n", socket->GetFd());
    bool b = co_await do_io(*socket);
    if (!b) {
      LOG_DEBUG("client close");
      break;
    }
    LOG_DEBUG("socket %d END!\n", socket->GetFd());
  }
}

coro::Task<> accept(net::Socket &listen_sock) {
  while (true) {
    auto socket = co_await listen_sock.accept();
    if (socket->GetFd() != -1) {
      do_io_loop(socket);
    }
  }
}

int main(int argc, char *argv[]) {
  signal(SIGPIPE, SIG_IGN);

  net::EventManager event_manager(1000, 0);
  net::Socket listen_sock{"127.0.0.1", 10009, event_manager};

  std::jthread t1([&listen_sock]() { accept(listen_sock); });

  event_manager.Start();
}
