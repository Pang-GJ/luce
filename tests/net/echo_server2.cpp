#include "luce/common/logger.h"
#include "luce/common/thread_pool.h"
#include "luce/io/io_awaiter.h"
#include "luce/net/tcp/tcp_connection.h"
#include "luce/net/tcp_all.h"

class EchoServer : public net::TcpApplication {
 private:
  co::Task<> OnRequest(net::TcpConnectionPtr conn,
                       net::TcpServer& server) override {
    while (true) {
      net::IOBuffer buffer(512);
      ssize_t recv_len = co_await conn->AsyncRead(&buffer);
      if (recv_len < 0) {
        LOG_ERROR("EchoServer read error");
        break;
      }
      if (recv_len == 0) {
        LOG_INFO("client closed");
        break;
      }
      LOG_DEBUG("Done send\n");
      auto res = co_await conn->AsyncWrite(buffer);
      if (res != recv_len) {
        LOG_ERROR("EchoServer write error");
      }
    }
  }
};

int main(int argc, char* argv[]) {
  net::InetAddress addr{12345};
  EchoServer app;
  net::TcpServer server(addr, &app, 8);
  server.Start();
  LOG_INFO("all down");
}
