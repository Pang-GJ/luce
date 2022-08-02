#include "InetAddress.h"
#include <sys/socket.h>

#include <cstring>

InetAddress::InetAddress(uint16_t port, const std::string &ipv4) {
  bzero(&addr_, sizeof addr_);
  addr_.sin_family = AF_INET;
  addr_.sin_port = htons(port);
  addr_.sin_addr.s_addr = inet_addr(ipv4.c_str());
}

InetAddress::InetAddress() {
  struct sockaddr_in addr;
  addr_ = addr;
}

std::string InetAddress::toIp() const {
  char buf[64] = {0};
  ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof buf);
  return buf;
}

std::string InetAddress::toIpPort() const {
  char buf[64] = {0};
  ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof buf);

  size_t end = strlen(buf);
  uint16_t port = ntohs(addr_.sin_port);

  sprintf(buf + end, ":%u", port);
  return buf;
}

uint16_t InetAddress::toPort() const { return ntohs(addr_.sin_port); }

const sockaddr *InetAddress::getSockAddr() const {
  return static_cast<const struct sockaddr *>(static_cast<const void *>(&addr_));
}

// 测试InetAddress
//
// #include <iostream>
//
// int main(int argc, char *argv[]) {
//   InetAddress addr(8080);
//   std::cout << addr.toIpPort() << std::endl;
//   return 0;
// }
