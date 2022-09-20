/*
 * spvm
 * @author 	: keith@robot9.me
 *
 */

#pragma once

#include "runtime.h"

namespace SPVM {

typedef struct DerivativeRet_ {
  SpvmValue *val[4];
  SpvmValue *dx[4];
  SpvmValue *dy[4];
} DerivativeRet;

/**
 *   p0(x,  y)--p1(x+1,  y)
 *   |          |
 *   p2(x,y+1)--p3(x+1,y+1)
 */
class RuntimeQuadContext {
 public:
  RuntimeQuadContext();
  ~RuntimeQuadContext();

  bool initWithModule(SpvmModule *module, SpvmWord heapSize);
  bool execEntryPoint(SpvmWord entryIdx = 0);

  SpvmValue *getDPdxFine(SpvmWord quadIdx, SpvmWord P);
  SpvmValue *getDPdyFine(SpvmWord quadIdx, SpvmWord P);
  SpvmValue *getDPdxCoarse(SpvmWord quadIdx, SpvmWord P);
  SpvmValue *getDPdyCoarse(SpvmWord quadIdx, SpvmWord P);

  inline SpvmValue *getDPdx(SpvmWord quadIdx, SpvmWord P) {
    return getDPdxFine(quadIdx, P);
  }

  inline SpvmValue *getDPdy(SpvmWord quadIdx, SpvmWord P) {
    return getDPdyFine(quadIdx, P);
  }

  inline SpvmWord getLocationByName(const char *name) {
    return rts_[0].getLocationByName(name);
  }

  inline SpvmUniformBinding getBindingByName(const char *name) {
    return rts_[0].getBindingByName(name);
  }
  inline void writeInput(void *data, SpvmWord location) {
    for (auto &rt : rts_) {
      rt.writeInput(data, location);
    }
  }
  inline void writeInputBuiltIn(void *data, SpvBuiltIn builtIn) {
    for (auto &rt : rts_) {
      rt.writeInputBuiltIn(data, builtIn);
    }
  }
  inline void writeUniformBinding(void *data, SpvmWord length, SpvmWord binding, SpvmWord set = 0) {
    for (auto &rt : rts_) {
      rt.writeUniformBinding(data, length, binding, set);
    }
  }
  inline void writeUniformPushConstants(void *data, SpvmWord length) {
    for (auto &rt : rts_) {
      rt.writeUniformPushConstants(data, length);
    }
  }
  inline void writeInput(SpvmWord quadIdx, void *data, SpvmWord location) {
    rts_[quadIdx].writeInput(data, location);
  }
  inline void writeInputBuiltIn(SpvmWord quadIdx, void *data, SpvBuiltIn builtIn) {
    rts_[quadIdx].writeInputBuiltIn(data, builtIn);
  }
  inline void writeUniformBinding(SpvmWord quadIdx, void *data, SpvmWord length, SpvmWord binding, SpvmWord set = 0) {
    rts_[quadIdx].writeUniformBinding(data, length, binding, set);
  }
  inline void writeUniformPushConstants(SpvmWord quadIdx, void *data, SpvmWord length) {
    rts_[quadIdx].writeUniformPushConstants(data, length);
  }

  inline void readOutput(SpvmWord quadIdx, void *data, SpvmWord location) {
    rts_[quadIdx].readOutput(data, location);
  }
  inline void readOutputBuiltIn(SpvmWord quadIdx, void *data, SpvBuiltIn builtIn) {
    rts_[quadIdx].readOutputBuiltIn(data, builtIn);
  }

 private:
  void evalDPdxFine(SpvmWord quadIdx, SpvmWord P);
  void evalDPdyFine(SpvmWord quadIdx, SpvmWord P);
  void evalDPdxCoarse(SpvmWord quadIdx, SpvmWord P);
  void evalDPdyCoarse(SpvmWord quadIdx, SpvmWord P);

  void syncQuadForResult(SpvmWord quadIdx, SpvmWord P);
  void storeDerivativeValue(SpvmValue **dst, void *val);

 private:
  Runtime rts_[4];
  SpvmModule *module_ = nullptr;
  DerivativeRet *dxdy_ = nullptr;
  SpvmWord heapSize_ = 0;
  SpvmByte *heap_ = nullptr;

  void **stackBase_ = nullptr;
  SpvmByte *sp_ = nullptr;
};

}