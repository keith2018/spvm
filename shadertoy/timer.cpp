/*
 * spvm-shadertoy
 * @author 	: keith@robot9.me
 *
 */

#include "timer.h"

namespace SPVM {
namespace ShaderToy {

void Timer::start() {
  start_ = std::chrono::steady_clock::now();
}

int64_t Timer::elapse() const {
  auto end = std::chrono::steady_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start_);
  return duration.count();
}

}
}