#pragma once

#include "luce/common/logger.hpp"
#include "luce/common/string_util.hpp"
#include "luce/net/http/http_context.hpp"

#include <string>
#include <unordered_map>

namespace net::http {

using HandleFunc = std::function<void(ContextPtr)>;

struct Router {
  void AddRouter(std::string_view method, std::string_view url,
                 const HandleFunc &handler) {
    if (method == "DELETE") {
      LOG_ERROR("could not DELETE now");
      return;
    }
    if (!EndsWith(url, "/")) {
      LOG_ERROR("register handler for %s: %s failed, url must ends with '/'",
                method.data(), url.data());
      return;
    }
    LOG_INFO("method: %s, url: %s", method.data(), url.data());
    auto key = Join({method, url}, "-");
    LOG_INFO("AddRouter key: %s", key.c_str());
    handlers_[key] = handler;
  }

  void Handle(const ContextPtr &ctx) {
    HandleFunc handler;
    bool ok = false;
    auto key = Join({ctx->method_, ctx->path_}, "-");
    if (handlers_.contains(key)) {
      handler = handlers_[key];
      ok = true;
    }
    if (ok) {
      handler(ctx);
    } else {
      ctx->HTML(404, Format("404 Not Found: {} not at {}\n", ctx->method_,
                            ctx->path_));
    }
  }

  std::unordered_map<std::string, HandleFunc> handlers_{};
};

}  // namespace net::http
