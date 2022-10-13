#include "common/Timestamp.h"

#include <ctime>

Timestamp::Timestamp() : microSecondsSinceEpoch_(0) {}

Timestamp::Timestamp(int64_t microSecondsSinceEpoch) : microSecondsSinceEpoch_(microSecondsSinceEpoch) {}

Timestamp Timestamp::now() { return Timestamp(time(nullptr)); }

std::string Timestamp::toString() const {
  char buf[128] = {0};
  tm *tm_time = localtime(&microSecondsSinceEpoch_);
  snprintf(buf, 128, "%4d/%02d/%02d %02d:%02d:%02d",
           tm_time->tm_year + 1900,  // 年要加1900
           tm_time->tm_mon + 1,      // 月要加1
           tm_time->tm_mday, tm_time->tm_hour, tm_time->tm_min, tm_time->tm_sec);

  return buf;
}

// 测试
// int main() {
//   std::cout << Timestamp::now().toString() << std::endl; 
//   return 0;
// }
