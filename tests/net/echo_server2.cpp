#include "luce/common/logger.hpp"
#include "luce/common/thread_pool.hpp"
#include "luce/io/io_awaiter.hpp"
#include "luce/net/tcp_all.hpp"

class EchoServer : public net::TcpApplication {
 private:
  co::Task<> OnRequest(net::TcpConnectionPtr conn,
                         net::TcpServer &server) override {
    while (true) {
      char buffer[512] = {0};
      ssize_t recv_len = co_await conn->read(buffer, sizeof buffer);
      ssize_t send_len = 0;
      while (send_len < recv_len) {
        ssize_t res = co_await conn->write(buffer, sizeof buffer);
        if (res <= 0) {
          co_return;
        }
        send_len += res;
      }
      LOG_DEBUG("Done send\n");
      if (recv_len <= 0) {
        co_return;
      }
      LOG_DEBUG("buffer: {}\n", buffer);
    }
  }
};

int main(int argc, char *argv[]) {
  net::InetAddress addr{12345};
  EchoServer app;
  net::TcpServer server(addr, &app, 8);
  server.Start();
  LOG_INFO("all down");
}
