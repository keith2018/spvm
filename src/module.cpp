/*
 * spvm
 * @author 	: keith@robot9.me
 *
 */

#include "module.h"

namespace SPVM {

SpvmModule::SpvmModule() : buffer(nullptr) {}

SpvmModule::~SpvmModule() {
  if (buffer) {
    delete[] buffer;
    buffer = nullptr;
  }
}

}
