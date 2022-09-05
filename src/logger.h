/*
 * spvm
 * @author 	: keith@robot9.me
 *
 */

#pragma once

#include <cstdio>
#include <cstring>
#include <cstdarg>

namespace SPVM {

#define LOG_SOURCE_LINE

#define SPVM_FILENAME (strrchr(__FILE__, '/') ? (strrchr(__FILE__, '/') + 1) : __FILE__)

#define LOGI(...) SPVM::Logger::log(SPVM::LOG_INFO,     SPVM_FILENAME, __LINE__, __VA_ARGS__)
#define LOGD(...) SPVM::Logger::log(SPVM::LOG_DEBUG,    SPVM_FILENAME, __LINE__, __VA_ARGS__)
#define LOGW(...) SPVM::Logger::log(SPVM::LOG_WARNING,  SPVM_FILENAME, __LINE__, __VA_ARGS__)
#define LOGE(...) SPVM::Logger::log(SPVM::LOG_ERROR,    SPVM_FILENAME, __LINE__, __VA_ARGS__)

static constexpr int MAX_LOG_LENGTH = 1024;

typedef void (*LogFunc)(void *context, int level, const char *msg);

enum LogLevel {
  LOG_INFO,
  LOG_DEBUG,
  LOG_WARNING,
  LOG_ERROR,
};

class Logger {
 public:
  static void setLogFunc(void *ctx, LogFunc func);
  static void setLogLevel(LogLevel level);
  static void log(LogLevel level, const char *file, int line, const char *message, ...);

 private:
  static void *logContext_;
  static LogFunc logFunc_;
  static LogLevel minLevel_;

  static char buf_[MAX_LOG_LENGTH];
};

}
