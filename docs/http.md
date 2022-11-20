# 类似gin的restful HTTP框架
## 简介
API设计参考`gin`，作为`luce`的应用示例，暂时不考虑性能优化。

## 功能实现
- GET
- POST
  - 简单POST报文解析，目前请求体的解析只支持了`param1=value1&param2=value2`这种格式
- PUT

## example
监听`/add/`，POST报文请求体中带有两个参数，返回这两个参数之和
```
  http_app.POST("/add/", [](const net::http::ContextPtr &ctx) {
    LOG_INFO("POST run: %s", ctx->req_->body_.c_str());

    auto param1 = ctx->QueryBody("param1");
    auto param2 = ctx->QueryBody("param2");
    if (param1.empty() || param2.empty()) {
      ctx->HTML(404, "Error Param");
      return;
    }
    auto res = atoi(param1.c_str()) + atoi(param2.c_str());
    ctx->HTML(200, Format("res: {}\n", res));
  });

```
