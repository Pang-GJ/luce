#include "common/logger.hpp"
#include "net/http_all.hpp"
#include "net/tcp_all.hpp"

int main(int argc, char *argv[]) {
  ThreadPool thread_pool(8);
  net::InetAddress addr{12345};
  net::http::HttpServer http_app;

  http_app.RegisterHandle("GET", "/",
                          [](const net::http::RequestPtr &request,
                             const net::http::ResponsePtr &response) {
                            LOG_INFO("Found a GET request");
                            response->SetHeader("Content-Type", "text/html");
                            response->SetStatusCode(200);
                            response->SetBody("<html>Hello</html>");
                          });

  http_app.RegisterHandle("POST", "/", [](const net::http::RequestPtr &request, const net::http::ResponsePtr &response) {
    LOG_INFO("Found a POST request");
    LOG_INFO("POST run: %s", request->body_.c_str());
    response->SetHeader("Content-Type", "text/html");
    response->SetStatusCode(200);
    response->SetBody("Hello");
  });

  net::TcpServer server(addr, thread_pool, &http_app);
  server.Start();
  LOG_INFO("all down");

  return 0;
}
