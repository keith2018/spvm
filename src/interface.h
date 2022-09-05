/*
 * spvm
 * @author 	: keith@robot9.me
 *
 */

#pragma once

#include <unordered_map>

#include "spvm.h"

namespace SPVM {

struct RuntimeContext;

typedef struct SpvmUniformBinding_ {
  SpvmWord set;
  SpvmWord binding;

  bool operator==(const struct SpvmUniformBinding_ &o) const {
    return o.set == set && o.binding == binding;
  }
} SpvmUniformBinding;

struct SpvmUniformBindingHash {
  SpvmWord operator()(const SpvmUniformBinding &o) const {
    return (std::hash<SpvmWord>()(o.set)) ^ (std::hash<SpvmWord>()(o.binding));
  }
};

class Interface {
 public:
  void init(RuntimeContext *ctx, void **resultIds);

  void writeInput(void *data, SpvmWord location);
  void writeInputBuiltIn(void *data, SpvBuiltIn builtIn);
  void writeUniformBinding(void *data, SpvmWord length, SpvmWord binding, SpvmWord set = 0);
  void writeUniformPushConstants(void *data, SpvmWord length);

  void readOutput(void *data, SpvmWord location);
  void readOutputBuiltIn(void *data, SpvBuiltIn builtIn);

 private:
  void checkDecorations(SpvmPointer *pointer, std::vector<SpvmDecoration> &decorations);
  void *writeValue(SpvmValue *value, void *data, SpvmWord length);
  void *readValue(SpvmValue *value, void *data);

 private:
  RuntimeContext *runtimeCtx_;
  void **resultIds_;

  std::unordered_map<SpvmWord, SpvmValue *> inputLocation_;
  std::unordered_map<SpvmWord, SpvmValue *> outputLocation_;
  std::unordered_map<SpvmWord, SpvmValue *> inoutBuiltIn_;
  std::unordered_map<SpvmUniformBinding, SpvmValue *, SpvmUniformBindingHash> bindings_;
  SpvmValue *pushConstants_ = nullptr;
};

}