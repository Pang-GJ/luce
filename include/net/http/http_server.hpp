#pragma once

#include "common/logger.hpp"
#include "coro/task.hpp"
#include "net/http/http_request.hpp"
#include "net/http/http_response.hpp"
#include "net/tcp_all.hpp"

#include <list>
#include <memory>
#include <string>

namespace net::http {

using RequestPtr = std::shared_ptr<HttpRequest>;
using ResponsePtr = std::shared_ptr<HttpResponse>;
using HandleType = std::function<void(RequestPtr, ResponsePtr)>;

class HttpServer : public TcpApplication {
  struct MethodType {
    std::string method_;
    HandleType handler_;
  };

 public:
  void RegisterHandle(std::string_view method, std::string_view url,
                      const HandleType &handler) {
    methods_[std::string(url)].emplace_back(
        MethodType{.method_ = std::string(method), .handler_ = handler});
  }

  void SetStaticPath(std::string_view path) {
    if (path.front() != '/') {
      LOG_FATAL("static path must start with '/'");
    }
    static_path_ = path;
  }

 private:
  coro::Task<> OnOpen(TcpConnectionPtr conn) override {
    LOG_INFO("connection open: fd = %d", conn->GetSocket()->GetFd());
    co_return;
  }
  coro::Task<> OnClose(TcpConnectionPtr conn) override {
    LOG_INFO("connection close: fd = %d", conn->GetSocket()->GetFd());
    co_return;
  }

  coro::Task<> OnRequest(TcpConnectionPtr conn, TcpServer &server) override;

  coro::Task<> SendFile(std::string_view path, RequestPtr request,
                        ResponsePtr response, TcpConnectionPtr conn);
  coro::Task<> SendResponse(ResponsePtr response, TcpConnectionPtr conn);

  std::unordered_map<std::string, std::list<MethodType>>
      methods_;  // path -> handle
  std::string static_path_;
  // std::string start_path_; // 程序启动的路径
};

}  // namespace net::http
