#include "common/logger.hpp"
#include "coro/hook_awaiters.hpp"
#include "coro/task.hpp"
#include "net/io_context.hpp"
#include "net/socket.hpp"

coro::Task<bool> inside_loop(net::Socket &socket) {
  char buffer[1024] = {0};
  ssize_t recv_len = co_await socket.recv(buffer, sizeof buffer);
  ssize_t send_len = 0;
  while (send_len < recv_len) {
    ssize_t res = co_await socket.send(buffer, sizeof buffer);
    if (res <= 0) {
      co_return false;
    }
    send_len = res;
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
    LOG_INFO("BEGIN!\n");
    bool b = co_await inside_loop(*socket);
    if (!b) {
      break;
    }
    LOG_INFO("END\n");
  }
}

coro::Task<> accept(net::Socket &listen) {
  while (true) {
    auto socket = co_await listen.accept();
    auto t = echo_socket(socket);
    t.resume();
  }
}

int main(int argc, char *argv[]) {
  net::IOContext io_context;
  net::Socket listen{"127.0.0.1", 10009, io_context};
  auto t = accept(listen);
  t.resume();

  io_context.Run();
}