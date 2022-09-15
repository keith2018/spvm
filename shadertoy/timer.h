/*
 * spvm-shadertoy
 * @author 	: keith@robot9.me
 *
 */

#pragma once

#include <chrono>
#include <string>

namespace SPVM {
namespace ShaderToy {

class Timer {
 public:
  Timer() = default;

  ~Timer() = default;

  void start();

  int64_t elapse() const;

 private:
  std::chrono::time_point<std::chrono::steady_clock> start_;
};

class ScopedTimer {
 public:

  explicit ScopedTimer(const char *str)
      : tag_(str) {
    timer_.start();
  }

  ~ScopedTimer() {
    printf("%s: %lld ms\n", tag_.c_str(), timer_.elapse());
  }

  explicit operator bool() {
    return true;
  }

 private:
  Timer timer_;
  std::string tag_;
};

#ifndef NDEBUG
#   define TIMED(X) if(SoftGL::ScopedTimer _ScopedTimer = (X))
#else
#   define TIMED(X)
#endif

}
}
