#pragma once

#include <arpa/inet.h>
#include <string>

// 封装socket地址类型
class InetAddress {
 public:
  explicit InetAddress(uint16_t port, const std::string &ipv4 = "127.0.0.1");

  explicit InetAddress(const struct sockaddr_in &addr) : addr_(addr) {}

  std::string toIp() const;
  std::string toIpPort() const;
  uint16_t toPort() const;

  const sockaddr *getSockAddr() const;

 private:
  struct sockaddr_in addr_;
};
