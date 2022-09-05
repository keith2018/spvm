/*
 * spvm
 * @author 	: keith@robot9.me
 *
 */

#pragma once

#include "spvm.h"

namespace SPVM {

class SpvmModule {
 public:
  SPVM::SpvmByte *buffer;

  SpvmByte versionMajor;
  SpvmByte versionMinor;
  SpvmWord generatorMagic;
  SpvmWord bound;
  SpvmWord instructionSchema;

  SpvmSource source;
  std::vector<SpvmExtension> sourceExtensions;
  std::vector<SpvmName> names;

  SpvmWord *code;
  SpvmWord codeSize;    // bytes

  std::vector<SpvCapability> capabilities;
  std::vector<SpvmExtension> extensions;
  SpvAddressingModel addressingModel;
  SpvMemoryModel memoryModel;
  std::vector<SpvmEntryPoint> entryPoints;
  std::vector<SpvmExecutionMode> executionModes;

  std::unordered_map<SpvmWord, std::vector<SpvmDecoration>> decorations;
  std::vector<SpvmWord> globalPointers;

 public:
  SpvmModule();
  ~SpvmModule();
};

}
