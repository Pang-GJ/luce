#include "luce/net/http/http_response.hpp"
#include "luce/common/string_util.hpp"

namespace net::http {

void HttpResponse::SetBody(std::string_view body) {
  buffer_.clear();
  buffer_ = http_version_ + " " + std::to_string(status_code_) + " " +
            GetCodeDescript(status_code_) + "\r\n";
  for (auto &header : headers_) {
    buffer_ += header.first + ":" + header.second + "\r\n";
  }
  buffer_ += "\r\n";
  buffer_ += body;
  buffer_ += "\r\n";
}

// HTTP状态码解释出处：https://www.runoob.com/http/http-status-codes.html
std::string HttpResponse::GetCodeDescript(int status_code) {
  std::string result;
  switch (status_code) {
    case 100: {
      // 继续。客户端应继续其请求
      result = "Continue";
      break;
    }
    case 101: {
      // 切换协议。服务器根据客户端的请求切换协议。只能切换到更高级的协议，例如，切换到HTTP的新版本协议
      result = "Switching Protocols";
      break;
    }
    case 200: {
      // 请求成功。一般用于GET与POST请求
      result = "OK";
      break;
    }
    case 201: {
      // 已创建。成功请求并创建了新的资源
      result = "Created";
      break;
    }
    case 202: {
      // 已接受。已经接受请求，但未处理完成
      result = "Accepted";
      break;
    }
    case 203: {
      // 非授权信息。请求成功。但返回的meta信息不在原始的服务器，而是一个副本
      result = "Non-Authoritative Information";
      break;
    }
    case 204: {
      // 无内容。服务器成功处理，但未返回内容。在未更新网页的情况下，可确保浏览器继续显示当前文档
      result = "No Content";
      break;
    }
    case 205: {
      // 重置内容。服务器处理成功，用户终端（例如：浏览器）应重置文档视图。可通过此返回码清除浏览器的表单域
      result = "Reset Content";
      break;
    }
    case 206: {
      // 部分内容。服务器成功处理了部分GET请求
      result = "Partial Content";
      break;
    }
    case 300: {
      // 多种选择。请求的资源可包括多个位置，相应可返回一个资源特征与地址的列表用于用户终端（例如：浏览器）选择
      result = "Multiple Choices";
      break;
    }
    case 301: {
      // 永久移动。请求的资源已被永久的移动到新URI，返回信息会包括新的URI，浏览器会自动定向到新URI。
      // 今后任何新的请求都应使用新的URI代替
      result = "Moved Permanently";
      break;
    }
    case 302: {
      // 临时移动。与301类似。但资源只是临时被移动。客户端应继续使用原有URI
      result = "Found";
      break;
    }
    case 303: {
      // 查看其它地址。与301类似。使用GET和POST请求查看
      result = "See Other";
      break;
    }
    case 304: {
      // 未修改。所请求的资源未修改，服务器返回此状态码时，不会返回任何资源。
      // 客户端通常会缓存访问过的资源，通过提供一个头信息指出客户端希望只返回在指定日期之后修改的资源
      result = "Not Modified";
      break;
    }
    case 305: {
      // 使用代理。所请求的资源必须通过代理访问
      result = "Use Proxy";
      break;
    }
    case 306: {
      // 已经被废弃的HTTP状态码
      result = "Unused";
      break;
    }
    case 307: {
      // 临时重定向。与302类似。使用GET请求重定向
      result = "Temporary Redirect";
      break;
    }
    case 400: {
      // 客户端请求的语法错误，服务器无法理解
      result = "Bad Request";
      break;
    }
    case 401: {
      // 请求要求用户的身份认证
      result = "Unauthorized";
      break;
    }
    case 403: {
      // 服务器理解请求客户端的请求，但是拒绝执行此请求
      result = "Forbidden";
      break;
    }
    case 404: {
      // 服务器无法根据客户端的请求找到资源（网页）。
      result = "Not Found";
      break;
    }
    case 405: {
      // 客户端请求中的方法被禁止
      result = "Method Not Allowed";
      break;
    }
    case 500: {
      // 服务器内部错误，无法完成请求
      result = "Internal Server Error";
      break;
    }
    case 501: {
      // 	服务器不支持请求的功能，无法完成请求
      result = "Not Implemented";
      break;
    }
    case 502: {
      // 	作为网关或者代理工作的服务器尝试执行请求时，从远程服务器接收到了一个无效的响应
      result = "Bad Gateway";
      break;
    }
    case 503: {
      // 	由于超载或系统维护，服务器暂时的无法处理客户端的请求。延时的长度可包含在服务器的Retry-After头信息中
      result = "Service Unavailable";
      break;
    }
    case 504: {
      // 充当网关或代理的服务器，未及时从远端服务器获取请求
      result = "Gateway Time-out";
      break;
    }
    case 505: {
      // 服务器不支持请求的HTTP协议的版本，无法完成处理
      result = "HTTP Version not supported";
      break;
    }
    default:
      result = "Unknow Code or Unsupported now";
  }
  return result;
}

std::string HttpResponse::GetFileType(std::string_view file_name) {
  auto type = Split(file_name, ".");
  if (type.size() != 2) {
    return "text/plain;charset=utf-8";
  }

  // string_view 不能使用switch？？
  if (type[1] == "html") {
    return "text/html; charset=utf-8";
  }
  if (type[1] == "jpg") {
    return "image/jpeg";
  }
  if (type[1] == "gif") {
    return "image/gif";
  }
  if (type[1] == "png") {
    return "image/png";
  }
  if (type[1] == "css") {
    return "text/css";
  }
  if (type[1] == "au") {
    return "audio/basic";
  }
  if (type[1] == "wav") {
    return "audio/wav";
  }
  if (type[1] == "avi") {
    return "video/x-msvideo";
  }
  if (type[1] == "mov") {
    return "video/quicktime";
  }
  if (type[1] == "mpeg") {
    return "video/mpeg";
  }
  if (type[1] == "vrml") {
    return "model/vrml";
  }
  if (type[1] == "midi") {
    return "audio/midi";
  }
  if (type[1] == "mp3") {
    return "audio/mpeg";
  }
  if (type[1] == "ogg") {
    return "application/ogg";
  }
  if (type[1] == "pac") {
    return "application/x-ns-proxy-autoconfig";
  }

  return "";
}

}  // namespace net::http
