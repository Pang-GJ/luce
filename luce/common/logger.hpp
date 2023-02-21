#pragma once

#include <cstdio>
#include <cstdlib>
#include <string>

#include "luce/common/noncopyable.h"

#ifdef USE_ORIGINAL
#define LOG_INFO(LogmsgFormat, ...)                   \
  do {                                                \
    Logger &logger = Logger::instance();              \
    logger.setLogLevel(INFO);                         \
    char buf[1024] = {0};                             \
    snprintf(buf, 1024, LogmsgFormat, ##__VA_ARGS__); \
    logger.log(buf);                                  \
  } while (0)

#define LOG_ERROR(LogmsgFormat, ...)                  \
  do {                                                \
    Logger &logger = Logger::instance();              \
    logger.setLogLevel(ERROR);                        \
    char buf[1024] = {0};                             \
    snprintf(buf, 1024, LogmsgFormat, ##__VA_ARGS__); \
    logger.log(buf);                                  \
  } while (0)

#define LOG_FATAL(LogmsgFormat, ...)                  \
  do {                                                \
    Logger &logger = Logger::instance();              \
    logger.setLogLevel(FATAL);                        \
    char buf[1024] = {0};                             \
    snprintf(buf, 1024, LogmsgFormat, ##__VA_ARGS__); \
    logger.log(buf);                                  \
    exit(-1);                                         \
  } while (0)

#ifdef LUCEDEBUG
#define LOG_DEBUG(LogmsgFormat, ...)                  \
  do {                                                \
    Logger &logger = Logger::instance();              \
    logger.setLogLevel(DEBUG);                        \
    char buf[1024] = {0};                             \
    snprintf(buf, 1024, LogmsgFormat, ##__VA_ARGS__); \
    logger.log(buf);                                  \
  } while (0)
#else
#define LOG_DEBUG(LogmsgFormat, ...)
#endif

// 定义日志的级别 INFO ERROR FATAL DEBUG
enum LogLevel {
  INFO,   // 普通信息
  ERROR,  // 错误信息
  FATAL,  // core信息
  DEBUG,  // 调试信息
};

// 输出一个日志类（单例模式）
class Logger : noncopyable {
 public:
  // 获取日志唯一的实例对象
  static Logger &instance();

  // 设置日志级别
  void setLogLevel(int Level);

  // 写日志的接口
  void log(const std::string &msg);

 private:
  Logger() = default;

  int logLevel_;
};
#else

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE//必须定义这个宏,才能输出文件名和行号
#include <spdlog/spdlog.h>

#define LOG_INFO(LogmsgFormat, ...)            \
  do {                                         \
    spdlog::info(LogmsgFormat, ##__VA_ARGS__); \
  } while (0)

#define LOG_DEBUG(LogmsgFormat, ...)            \
  do {                                          \
    spdlog::debug(LogmsgFormat, ##__VA_ARGS__); \
  } while (0)

#define LOG_ERROR(LogmsgFormat, ...)            \
  do {                                          \
    spdlog::error(LogmsgFormat, ##__VA_ARGS__); \
  } while (0)

#define LOG_FATAL(LogmsgFormat, ...)           \
  do {                                         \
    spdlog::warn(LogmsgFormat, ##__VA_ARGS__); \
    exit(-1);                                  \
  } while (0)

#endif
