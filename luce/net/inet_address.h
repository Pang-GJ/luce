#pragma once

#include <arpa/inet.h>
#include <cstdint>
#include <cstring>
#include <string_view>

namespace net {
class InetAddress {
 public:
  explicit InetAddress(uint16_t port, std::string_view ipv4 = "127.0.0.1") {
    bzero(&addr_, sizeof addr_);
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);
    addr_.sin_addr.s_addr = inet_addr(ipv4.data());
  }

  explicit InetAddress(sockaddr_in addr) : addr_(addr) {}

  void SetSockAddr(const sockaddr_in& addr) { addr_ = addr; }

  struct sockaddr* GetSockAddr() const { return (struct sockaddr*)&addr_; }

 private:
  sockaddr_in addr_{};
};

}  // namespace net