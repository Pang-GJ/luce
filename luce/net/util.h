#pragma once

#include <netinet/in.h>
#include <cstdint>
#include <cstring>

namespace net {

int32_t GetInt32ForNetByte(const char* buf) {
  int32_t res;
  std::memcpy(&res, buf, sizeof(res));
  return static_cast<int32_t>(ntohl(res));
}

int32_t GetNetInt32(int32_t num) { return static_cast<int32_t>(htonl(num)); }

}  // namespace net