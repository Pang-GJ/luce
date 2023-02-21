#include <string>
#include "luce/common/json.hpp"
#include "luce/common/logger.hpp"
#include "luce/net/http_all.hpp"
#include "luce/net/tcp_all.hpp"

int main(int argc, char *argv[]) {
  ThreadPool thread_pool(8);
  net::InetAddress addr{12345};
  net::http::HttpServer http_app;

  http_app.GET("/ping/", [](const net::http::ContextPtr &ctx) {
    ctx->HTML(200, "Pong");
  });

  http_app.POST("/add/", [](const net::http::ContextPtr &ctx) {
    LOG_INFO("POST run: {}", ctx->req_->body_.c_str());

    auto param1 = ctx->QueryBody("param1");
    auto param2 = ctx->QueryBody("param2");
    if (param1.empty() || param2.empty()) {
      ctx->HTML(404, "Error Param");
      return;
    }
    auto res = atoi(param1.c_str()) + atoi(param2.c_str());
    ctx->HTML(200, Format("res: {}\n", res));
  });

  net::TcpServer server(addr, thread_pool, &http_app);
  server.Start();
  LOG_INFO("all down");

  return 0;
}
