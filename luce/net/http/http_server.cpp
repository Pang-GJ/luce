#include <fcntl.h>
#include <unistd.h>
#include <cstdlib>

#include "luce/common/file_util.hpp"
#include "luce/common/logger.hpp"
#include "luce/common/singleton.hpp"
#include "luce/io/io_awaiter.hpp"
#include "luce/net/http/http_response.hpp"
#include "luce/net/http/http_server.hpp"
#include "luce/timer/timer.hpp"

namespace net::http {

co::Task<> HttpServer::OnRequest(TcpConnectionPtr conn, TcpServer &server) {
  for (;;) {
    char buffer[MAX_REQUEST_SIZE] = {0};
    auto timer_id =
        Singleton<timer::TimerManager>::GetInstance()->AddTimer(1000, [=] {
          LOG_DEBUG("timer work ");
          conn->Close();
        });
    auto recv_len = co_await conn->read(&buffer, sizeof buffer);
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
    http_request->Parse(buffer);

    // 是否未读完
    if (http_request->headers_.contains("Content-Length")) {
      auto content_len = std::stoi(http_request->headers_["Content-Length"]);
      while (http_request->body_.size() != content_len) {
        bzero(&buffer, sizeof buffer);
        recv_len = co_await conn->read(&buffer, sizeof buffer);
        if (recv_len < 0) {
          co_return;
        }
        buffer[recv_len] = '\0';
        http_request->body_ += buffer;
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
      // LOG_DEBUG("Connection: {}", http_request->headers_["Connection"].c_str());
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
  std::string buffer;
  buffer.resize(file_size);
  ssize_t ret = 0;
  while ((ret = read(fd, &*buffer.begin(), buffer.size())) > 0) {
    co_await conn->write(&*buffer.begin(), file_size);
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
  ssize_t send_size = 0;
  while (total_size > 0) {
    auto res = co_await conn->write(&*(response->GetData().begin() + send_size),
                                    total_size);
    if (res <= 0) {
      break;
    }
    total_size -= res;
    send_size += res;
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
