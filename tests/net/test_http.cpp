#include "common/logger.hpp"
#include "common/json.hpp"
#include "net/http_all.hpp"
#include "net/tcp_all.hpp"

int main(int argc, char *argv[]) {
  ThreadPool thread_pool(8);
  net::InetAddress addr{12345};
  net::http::HttpServer http_app;

  http_app.RegisterHandle("GET", "/",
                          [](const net::http::RequestPtr &request,
                             const net::http::ResponsePtr &response) {
                            response->SetHeader("Content-Type", "text/html");
                            response->SetStatusCode(200);
                            response->SetBody("<html>Hello</html>");
                          });

  http_app.RegisterHandle("POST", "/",
                          [](const net::http::RequestPtr &request,
                             const net::http::ResponsePtr &response) {
                            LOG_INFO("POST run: %s", request->body_.c_str());
                            response->SetHeader("Content-Type", "text/html");
                            response->SetStatusCode(200);
                            response->SetBody("Hello");
                          });

  http_app.POST("/add/", [](const net::http::RequestPtr &request,
                           const net::http::ResponsePtr &response) {
    LOG_INFO("POST run: %s", request->body_.c_str());
    response->SetHeader("Content-Type", "text/html");
    if (request->body_params_.size() < 2) {
      LOG_INFO("body_param size: %lu", request->body_params_.size());
      response->SetStatusCode(404);
      response->SetBody("Error Param");
      return;
    }
    response->SetStatusCode(200);
    auto param1 = atol(request->body_params_["param1"].c_str());
    auto param2 = atol(request->body_params_["param2"].c_str());
    auto res = param1 + param2;
    response->SetBody(std::to_string(res));
  });

  net::TcpServer server(addr, thread_pool, &http_app);
  server.Start();
  LOG_INFO("all down");

  return 0;
}
