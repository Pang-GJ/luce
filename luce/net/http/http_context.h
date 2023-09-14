#pragma once

#include "luce/common/json.h"
#include "luce/common/logger.h"
#include "luce/common/string_util.h"
#include "luce/net/http/http_request.h"
#include "luce/net/http/http_response.h"

#include <memory>
#include <string_view>

namespace net::http {

struct HttpContext {
  HttpContext(RequestPtr req, ResponsePtr res)
      : req_(std::move(req)),
        res_(std::move(res)),
        path_(req_->url_),
        method_(req_->method_) {}

  std::string QueryURL(const std::string& key) {
    if (req_->url_params_.contains(key)) {
      return req_->url_params_[key];
    }
    return {};
  }

  std::string QueryBody(const std::string& key) {
    if (req_->body_params_.contains(key)) {
      return req_->body_params_[key];
    }
    return {};
  }

  void SetHeader(std::string_view key, std::string_view value) {
    res_->SetHeader(key, value);
  }

  void Status(int code) {
    status_code_ = code;
    res_->SetStatusCode(code);
  }

  template <typename... Args>
  void String(int code, std::string_view fmt, Args&&... args) {
    SetHeader("Content-Type", "text/plain");
    Status(code);
    res_->SetBody(String::Format(fmt, std::forward<Args>(args)...));
  }

  void JSON(int code, tinyjson::JObject& json_object) {
    SetHeader("Content-Type", "application/json");
    Status(code);
    // TODO(pgj): impl json
    // res_->SetBody(tinyjson::Parser::ToJSON(json_object));
    LOG_ERROR("ctx.JSON didn't impl");
  }

  void HTML(int code, std::string_view html) {
    SetHeader("Content-Type", "text/html");
    Status(code);
    res_->SetBody(html);
  }

  // request and response
  RequestPtr req_;
  ResponsePtr res_;

  // request info
  std::string path_;
  std::string method_;

  // response info
  int status_code_{500};
};

using ContextPtr = std::shared_ptr<HttpContext>;

}  // namespace net::http
