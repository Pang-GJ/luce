#pragma once

#include "luce/co/task.h"
#include "luce/common/logger.h"
#include "luce/common/string_util.h"
#include "luce/net/http/http_context.h"
#include "luce/net/http/http_request.h"
#include "luce/net/http/http_response.h"
#include "luce/net/http/http_router.h"
#include "luce/net/tcp_all.h"

#include <list>
#include <memory>
#include <string>

namespace net::http {

class HttpServer : public TcpApplication {
 public:
  void GET(std::string_view url, const HandleFunc& handler) {
    router_.AddRouter("GET", url, handler);
  }

  void POST(std::string_view url, const HandleFunc& handler) {
    router_.AddRouter("POST", url, handler);
  }

  void PUT(std::string_view url, const HandleFunc& handler) {
    router_.AddRouter("PUT", url, handler);
  }

  void DELETE(std::string_view url, const HandleFunc& handler) {
    router_.AddRouter("DELETE", url, handler);
  }

  void SetStaticPath(std::string_view path) {
    if (path.front() != '/') {
      LOG_FATAL("static path must start with '/'");
    }
    static_path_ = path;
  }

 private:
  co::Task<> OnRequest(TcpConnectionPtr conn, TcpServer& server) override;

  co::Task<> ServerHTTP(TcpConnectionPtr conn, RequestPtr http_request,
                        ResponsePtr http_response);

  co::Task<> SendFile(std::string_view path, RequestPtr request,
                      ResponsePtr response, TcpConnectionPtr conn);
  co::Task<> SendResponse(ResponsePtr response, TcpConnectionPtr conn);

  Router router_;  // method-url -> handle
  std::string static_path_;
  // std::string start_path_; // 程序启动的路径
};

}  // namespace net::http
