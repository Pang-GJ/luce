#include "luce/common/logger.hpp"
#include "luce/common/thread_pool.hpp"
#include "luce/io/io_awaiter.hpp"
#include "luce/net/tcp_all.hpp"

class EchoServer : public net::TcpApplication {
 private:
  coro::Task<> OnRequest(net::TcpConnectionPtr conn,
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
      LOG_DEBUG("buffer: %s\n", buffer);
    }
  }

  coro::Task<> OnOpen(net::TcpConnectionPtr conn) override {
    //    LOG_INFO("sockfd: %d open", conn->GetSocket()->GetFd());
    co_return;
  }

  coro::Task<> OnClose(net::TcpConnectionPtr conn) override {
    //    LOG_INFO("sockfd: %d close", conn->GetSocket()->GetFd());
    co_return;
  }
};

int main(int argc, char *argv[]) {
  ThreadPool thread_pool(8);
  net::InetAddress addr{12345};
  EchoServer app;
  net::TcpServer server(addr, thread_pool, &app);
  server.Start();
  LOG_INFO("all down");
}
