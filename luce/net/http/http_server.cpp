#include <fcntl.h>
#include <unistd.h>
#include <cstdlib>

#include "luce/common/file_util.hpp"
#include "luce/common/logger.hpp"
#include "luce/common/singleton.hpp"
#include "luce/io/io_awaiter.hpp"
#include "luce/net/http/http_request.hpp"
#include "luce/net/http/http_response.hpp"
#include "luce/net/http/http_server.hpp"
#include "luce/net/tcp/tcp_connection.hpp"
#include "luce/timer/timer.hpp"

namespace net::http {

co::Task<> HttpServer::OnRequest(TcpConnectionPtr conn, TcpServer &server) {
  for (;;) {
    IOBuffer buffer(MAX_REQUEST_SIZE);
    auto timer_id =
        Singleton<timer::TimerManager>::GetInstance()->AddTimer(1000, [=] {
          LOG_DEBUG("timer work ");
          conn->Close();
        });
    auto recv_len = co_await conn->AsyncRead(&buffer);
    if (recv_len < 0) {
      break;
    }
    if (recv_len == 0) {
      continue;
    }
    // 截断buffer
    buffer[recv_len] = '\0';

    // 解析请求
    auto http_request = std::make_shared<HttpRequest>();
    http_request->Parse(buffer.data());

    // 是否未读完
    if (http_request->headers_.contains("Content-Length")) {
      auto content_len = std::stoi(http_request->headers_["Content-Length"]);
      while (http_request->body_.size() != content_len) {
        buffer.resize(MAX_REQUEST_SIZE);
        recv_len = co_await conn->AsyncRead(&buffer);
        if (recv_len < 0) {
          co_return;
        }
        buffer[recv_len] = '\0';
        http_request->body_ += buffer.data();
      }
    }
    Singleton<timer::TimerManager>::GetInstance()->RemoveTimer(timer_id);

    // http_request->Debug();
    // LOG_INFO("got a {} request on {}", http_request->method_.c_str(),
    //          http_request->url_.c_str());

    auto http_response = std::make_shared<HttpResponse>();
    http_response->SetHTTPVersion(http_request->http_version_);

    // 运行Router回调
    co_await ServerHTTP(conn, http_request, http_response);

    // TODO(pgj): check keep alive
    if (http_request->headers_.contains("Connection")) {
      // LOG_DEBUG("Connection: {}",
      // http_request->headers_["Connection"].c_str());
      if (http_request->headers_["Connection"] == "close") {
        break;
      }
    }
    // break;
  }
  co_return;
}

co::Task<> HttpServer::SendFile(std::string_view path, RequestPtr request,
                                ResponsePtr response, TcpConnectionPtr conn) {
  response->SetHeader("Content-Type:", response->GetFileType(path));
  auto file_size = FileSize(path);
  response->SetHeader("Content-Length:", std::to_string(file_size));
  response->SetBody("\r\n");
  co_await SendResponse(response, conn);

  // 发送文件
  int fd = open(path.data(), O_RDONLY);
  if (fd == -1) {
    LOG_ERROR("open file failed");
    co_return;
  }
  IOBuffer buffer;
  buffer.resize(file_size);
  ssize_t ret = 0;
  while ((ret = read(fd, &*buffer.begin(), buffer.size())) > 0) {
    co_await conn->AsyncWrite(buffer);
  }
  if (ret == -1) {
    LOG_ERROR("read file failed");
    co_return;
  }
  close(fd);
}

co::Task<> HttpServer::SendResponse(ResponsePtr response,
                                    TcpConnectionPtr conn) {
  // 先发送请求头
  auto total_size = response->GetData().size();
  IOBuffer buffer(response->GetData().begin(), response->GetData().end());
  auto res = co_await conn->AsyncWrite(buffer);
  if (res != total_size) {
    LOG_ERROR("HttpServer::SendResponse, did'nt finish, send: {}, total: {}",
              res, total_size);
  }
  response->Clear();
}

co::Task<> HttpServer::ServerHTTP(TcpConnectionPtr conn,
                                  RequestPtr http_request,
                                  ResponsePtr http_response) {
  router_.Handle(std::make_shared<HttpContext>(http_request, http_response));
  // LOG_INFO("debug -> HTTP Response: {}", http_response->GetData().c_str());
  co_await SendResponse(http_response, conn);
}

}  // namespace net::http
