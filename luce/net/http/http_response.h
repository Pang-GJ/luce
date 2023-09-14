#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

namespace net::http {

class HttpResponse {
 public:
  // could only call once, otherwise error
  void SetBody(std::string_view body);

  void SetStatusCode(int status_code) { status_code_ = status_code; }
  void SetHeader(std::string_view key, std::string_view value) {
    headers_[std::string(key)] = value;
  }
  void SetHTTPVersion(std::string_view http_version) {
    http_version_ = http_version;
  }

  std::string GetFileType(std::string_view file_name);

  std::string& GetData() { return buffer_; }

  void Clear() { buffer_.clear(); }

 private:
  std::string GetCodeDescript(int status_code);

  // TODO(pgj): impl high performance buffer
  std::string buffer_;
  int status_code_;
  std::string http_version_;  // it comes from HttpRequset
  std::unordered_map<std::string, std::string> headers_;
  // bool finish_{false};  // 相应数据是否完成
};

using ResponsePtr = std::shared_ptr<HttpResponse>;

}  // namespace net::http
