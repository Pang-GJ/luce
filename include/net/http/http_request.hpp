#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "common/noncopyable.h"

namespace net::http {

// 单次最大请求报文大小限制 64K
#define MAX_REQUEST_SIZE (1024 * 64)

struct HttpRequest : noncopyable {
  void Parse(std::string_view data);

  void Debug();

  std::string method_;        // GET or POST
  std::string url_;           // eg: /foo?name=pgj
  std::string uri_;           // eg: /foo
  std::string http_version_;  // 1.1 or 1.0
  std::string body_;
  std::unordered_map<std::string, std::string> headers_;
  std::unordered_map<std::string, std::string> url_params_;
  std::unordered_map<std::string, std::string> body_params_;

 private:
  // 解析请求头
  void ParseHeaders(std::vector<std::string_view> &headers);
  // 解析请求行
  void ParseLine(std::string_view line);
  // 解析请求行中的URL
  void ParseURLParams(std::string_view url);
  // 简单的解析请求体
  void ParseBody(std::string_view body);
};

}  // namespace net::http
