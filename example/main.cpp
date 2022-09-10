/*
 * spvm
 * @author 	: keith@robot9.me
 *
 */

#include <iostream>

#include "decoder.h"
#include "runtime.h"

#define HEAP_SIZE 128 * 1024
const char *SPV_PATH = "shaders/simple.frag.spv";

int main(int argc, char *argv[]) {
  SPVM::SpvmModule module;
  SPVM::Runtime runtime;

  // decode spir-v file
  bool success = SPVM::Decoder::decodeFile(SPV_PATH, &module);
  if (!success) {
    std::cout << "error decode spir-v file";
    return -1;
  }

  // init runtime
  success = runtime.initWithModule(&module, HEAP_SIZE);
  if (!success) {
    std::cout << "error init module";
    return -1;
  }

  // get uniform locations
  SPVM::SpvmWord inColorLoc = runtime.getLocationByName("inColor");
  SPVM::SpvmWord outFragColorLoc = runtime.getLocationByName("outFragColor");

  // write input
  float inColor[3]{0.2f, 0.3f, 0.4f};
  runtime.writeInput(inColor, inColorLoc);

  // execute shader entry function 'main'
  success = runtime.execEntryPoint();
  if (!success) {
    std::cout << "error exec entrypoint function";
    return -1;
  }

  // read output
  float outFragColor[4];
  runtime.readOutput(outFragColor, outFragColorLoc);

  std::cout << "outFragColor[0]: " << outFragColor[0] << std::endl;
  std::cout << "outFragColor[1]: " << outFragColor[1] << std::endl;
  std::cout << "outFragColor[2]: " << outFragColor[2] << std::endl;
  std::cout << "outFragColor[3]: " << outFragColor[3] << std::endl;

  return 0;
}