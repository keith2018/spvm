/*
 * spvm
 * @author 	: keith@robot9.me
 *
 */

#pragma once

#include <cstdlib>
#include "spvm.h"
#include "module.h"

namespace SPVM {

class Decoder {
 public:
  static bool decodeFile(const char *path, SpvmModule *module);
  static bool decodeBytes(const SpvmByte *bytes, SpvmWord length, SpvmModule *module);

 private:
  static bool checkEndian(const SpvmWord *ptr);
  static bool checkDerivativeOpcodes(SpvmOpcode opcode);
};

}
