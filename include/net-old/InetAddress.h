#pragma once

#include <arpa/inet.h>
#include <string>

namespace luce::net {

// 封装socket地址类型
class InetAddress {
 public:
  explicit InetAddress(uint16_t port, const std::string &ipv4 = "127.0.0.1");

  explicit InetAddress(const struct sockaddr_in &addr) : addr_(addr) {}

  // 默认构造函数，尽量不要调用，目前在Acceptor::handleRead()中调用了
  InetAddress();

  std::string toIp() const;
  std::string toIpPort() const;
  uint16_t toPort() const;

  const sockaddr *getSockAddr() const;
  void setSockAddr(const sockaddr_in &addr) { addr_ = addr; }

 private:
  struct sockaddr_in addr_;
};

}  // namespace luce::net