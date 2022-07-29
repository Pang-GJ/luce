#include "Logger.h"

#include <iostream>

Logger &Logger::instance() {
  static Logger logger;
  return logger;
}

void Logger::setLogLevel(int Level) { logLevel_ = Level; }

// 写日志 格式：[日志级别] time : msg
void Logger::log(std::string &msg) {
  switch (logLevel_) {
    case INFO:
      std::cout << "[INFO]";
      break;

    case ERROR:
      std::cout << "[ERROR]";
      break;

    case FATAL:
      std::cout << "[FATAL]";
      break;

    case DEBUG:
      std::cout << "[DEBUG]";
      break;

    default:
      break;
  }

  // 打印时间和msg
  std::cout << "print time"
            << " : " << msg << std::endl;
}
