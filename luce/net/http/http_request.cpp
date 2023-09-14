#include "luce/net/http/http_request.h"
#include "luce/common/logger.h"
#include "luce/common/string_util.h"

namespace net::http {

void HttpRequest::Parse(std::string_view data) {
  // 分割出请求头和请求体
  auto head_and_body = String::Split(data, "\r\n\r\n", true);
  if (head_and_body.size() != 2) {
    LOG_ERROR(
        "HTTP Request Data Error, dosen't have RequestBody, the split data "
        "size: {}",
        head_and_body.size());
  }
  if (!head_and_body[1].empty()) {
    this->body_ = head_and_body[1];
  }

  // 分割出请求行和请求头
  auto line_and_head = String::Split(head_and_body[0], "\r\n", false);
  // 解析请求行
  ParseLine(line_and_head[0]);
  // 解析请求头
  ParseHeaders(line_and_head);
  // TODO(pgj): 解析请求体
  ParseBody(head_and_body[1]);
}

// 解析请求头
void HttpRequest::ParseHeaders(std::vector<std::string_view>& headers) {
  for (size_t i = 1; i < headers.size(); ++i) {
    if (!headers[i].empty() && String::Contains(headers[i], ":")) {
      auto key_value = String::Split(headers[i], ':', false);
      if (key_value.size() == 2) {
        // 去除空白
        std::string key{key_value[0]};
        std::string value{key_value[1]};
        String::Trim(key);
        String::Trim(value);
        this->headers_[key] = value;
      }
    }
  }
}

// 解析请求行
void HttpRequest::ParseLine(std::string_view line) {
  auto line_datas = String::Split(line, ' ');
  if (line_datas.size() != 3) {
    LOG_ERROR("HTTP Request Line error, it's size != 3");
  }
  this->method_ = line_datas[0];
  this->url_ = line_datas[1];
  this->http_version_ = line_datas[2];

  // 判断有没有参数
  bool has_url_params = false;
  auto url_datas = String::Split(line_datas[1], '&');
  if (url_datas.size() == 2) {
    has_url_params = true;
    this->uri_ = std::string{url_datas[0]};
  } else {
    this->uri_ = this->url_;
  }

  if (has_url_params) {
    ParseURLParams(url_datas[1]);
  }
}

// 解析请求行中的URL
void HttpRequest::ParseURLParams(std::string_view url) {
  auto params = String::Split(url, '&');
  for (auto param : params) {
    auto key_value = String::Split(param, '=');
    if (key_value.size() == 2) {
      // 去除空白
      std::string key{key_value[0]};
      std::string value{key_value[1]};
      String::Trim(key);
      String::Trim(value);
      this->url_params_[key] = value;
    }
  }
}

void HttpRequest::ParseBody(std::string_view body) {
  auto params = String::Split(body, '&');
  for (auto param : params) {
    auto key_value = String::Split(param, '=');
    if (key_value.size() == 2) {
      // 去除空白
      std::string key{key_value[0]};
      std::string value{key_value[1]};
      String::Trim(key);
      String::Trim(value);
      this->body_params_[key] = value;
    }
  }
}

void HttpRequest::Debug() {
  LOG_INFO("method: {}", method_.c_str());
  LOG_INFO("http version: {}", http_version_.c_str());
  LOG_INFO("url: {}", url_.c_str());
  LOG_INFO("uri: {}", uri_.c_str());
  LOG_INFO("body: {}", body_.c_str());
}

}  // namespace net::http
