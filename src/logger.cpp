/*
 * spvm
 * @author 	: keith@robot9.me
 *
 */

#include "logger.h"

namespace SPVM {

void *Logger::logContext_ = nullptr;
LogFunc Logger::logFunc_ = nullptr;

#if DEBUG
LogLevel Logger::minLevel_ = LOG_INFO;
#else
LogLevel Logger::minLevel_ = LOG_WARNING;
#endif

char Logger::buf_[MAX_LOG_LENGTH] = {};

void Logger::setLogFunc(void *ctx, LogFunc func) {
  logContext_ = ctx;
  logFunc_ = func;
}

void Logger::setLogLevel(LogLevel level) {
  minLevel_ = level;
}

void Logger::log(LogLevel level, const char *file, int line, const char *message, ...) {
  if (level < minLevel_) {
    return;
  }

  va_list argPtr;
  va_start(argPtr, message);
  vsnprintf(buf_, MAX_LOG_LENGTH - 1, message, argPtr);
  va_end(argPtr);
  buf_[MAX_LOG_LENGTH - 1] = '\0';

  if (logFunc_ != nullptr) {
    logFunc_(logContext_, level, buf_);
    return;
  }

  switch (level) {
#ifdef LOG_SOURCE_LINE
    case LOG_INFO:    fprintf(stdout, "[SPVM][INFO] %s:%d: %s\n", file, line, buf_);    break;
    case LOG_DEBUG:   fprintf(stdout, "[SPVM][DEBUG] %s:%d: %s\n", file, line, buf_);   break;
    case LOG_WARNING: fprintf(stdout, "[SPVM][WARNING] %s:%d: %s\n", file, line, buf_); break;
    case LOG_ERROR:   fprintf(stdout, "[SPVM][ERROR] %s:%d: %s\n", file, line, buf_);   break;
#else
    case LOG_INFO:    fprintf(stdout, "[SPVM][INFO] : %s\n", buf_);    break;
    case LOG_DEBUG:   fprintf(stdout, "[SPVM][DEBUG] : %s\n", buf_);   break;
    case LOG_WARNING: fprintf(stdout, "[SPVM][WARNING] : %s\n", buf_); break;
    case LOG_ERROR:   fprintf(stdout, "[SPVM][ERROR] : %s\n", buf_);   break;
#endif
  }
  fflush(stdout);
}

}