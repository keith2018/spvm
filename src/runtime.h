/*
 * spvm
 * @author 	: keith@robot9.me
 *
 */

#pragma once

#include "spvm.h"
#include "module.h"
#include "interface.h"

#define SPVM_OP_DISPATCH_TAIL_CALL

namespace SPVM {

typedef struct SpvmFrame {
  struct SpvmFrame *prevFrame;
  SpvmWord paramsIdx;
  void **params;
  SpvmValue *resultValue;
  SpvmWord parentBlockId;
  SpvmWord currBlockId;
  SpvmWord *pc;
} SpvmFrame;

class Runtime;

typedef enum {
  Result_NoError = 0,
  Result_Unreachable,
  Result_Killed,
  Result_Error,
  Result_InitEnd,
  Result_FunctionEnd
} RuntimeResult;

typedef struct RuntimeContext {
  Runtime *runtime;
  SpvmModule *module;
  SpvmResult *results;
  SpvmByte *stackBase;
  SpvmFrame *currFrame;
#ifndef SPVM_OP_DISPATCH_TAIL_CALL
  SpvmWord *pc;
  SpvmByte *sp;
#endif
} RuntimeContext;

class Runtime {
 public:
  Runtime();
  ~Runtime();
  bool initWithModule(SpvmModule *module, SpvmWord heapSize);
  bool execEntryPoint(SpvmWord entryIdx = 0);

  inline SpvmWord getLocationByName(const char *name) {
    return interface_.getLocationByName(name);
  }

  inline SpvmUniformBinding getBindingByName(const char *name) {
    return interface_.getBindingByName(name);
  }

  inline void writeInput(void *data, SpvmWord location) {
    interface_.writeInput(data, location);
  }
  inline void writeInputBuiltIn(void *data, SpvBuiltIn builtIn) {
    interface_.writeInputBuiltIn(data, builtIn);
  }
  inline void writeUniformBinding(void *data, SpvmWord length, SpvmWord binding, SpvmWord set = 0) {
    interface_.writeUniformBinding(data, length, binding, set);
  }
  inline void writeUniformPushConstants(void *data, SpvmWord length) {
    interface_.writeUniformPushConstants(data, length);
  }
  inline void readOutput(void *data, SpvmWord location) {
    interface_.readOutput(data, location);
  }
  inline void readOutputBuiltIn(void *data, SpvBuiltIn builtIn) {
    interface_.readOutputBuiltIn(data, builtIn);
  }

 private:
  RuntimeResult invokeFunction(SpvmWord funcId);

 private:
  RuntimeContext ctx_;
  Interface interface_;
  SpvmWord heapSize_;
  SpvmByte *heap_;  // [resultIds][types, global vars][frame0, frame0 vars][frame1, frame1 vars] ...
};

}
