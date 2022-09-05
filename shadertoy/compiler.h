/*
 * spvm-shadertoy
 * @author 	: keith@robot9.me
 *
 */

#pragma once

#include <vector>
#include <cstdint>

namespace SPVM {
namespace ShaderToy {

class Compiler {
 public:
  static std::vector<uint32_t> compileShaderFragment(const char *shaderSource);

};

}
}