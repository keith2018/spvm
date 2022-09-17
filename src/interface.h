/*
 * spvm
 * @author 	: keith@robot9.me
 *
 */

#pragma once

#include <vector>
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
  void init(RuntimeContext *ctx);

  SpvmWord getLocationByName(const char *name);
  SpvmUniformBinding getBindingByName(const char *name);

  void writeInput(void *data, SpvmWord location);
  void writeInputBuiltIn(void *data, SpvBuiltIn builtIn);
  void writeUniformBinding(void *data, SpvmWord length, SpvmWord binding, SpvmWord set = 0);
  void writeUniformPushConstants(void *data, SpvmWord length);

  void readOutput(void *data, SpvmWord location);
  void readOutputBuiltIn(void *data, SpvBuiltIn builtIn);

 private:
  void checkDecorations(SpvmPointer *pointer, SpvmWord pointerId, std::vector<SpvmDecoration> &decorations);
  void *writeValue(SpvmValue *value, void *data, SpvmWord length);
  void *readValue(SpvmValue *value, void *data);

 private:
  RuntimeContext *runtimeCtx_;

  std::unordered_map<SpvmWord, SpvmValue *> inputLocation_;   // <Location, SpvmValue(ptr)>
  std::unordered_map<SpvmWord, SpvmValue *> outputLocation_;  // <Location, SpvmValue(ptr)>
  std::unordered_map<SpvmWord, SpvmValue *> inoutBuiltIn_;    // <BuiltIn, SpvmValue(ptr)>
  std::unordered_map<SpvmUniformBinding, SpvmValue *, SpvmUniformBindingHash> bindings_;  // <Binding, SpvmValue>
  SpvmValue *pushConstants_ = nullptr;

  std::unordered_map<SpvmWord, SpvmWord> locationMap_;            // <SpvmValue(ResultId), Location>
  std::unordered_map<SpvmWord, SpvmUniformBinding> bindingMap_;   // <SpvmValue(ResultId), Binding>
};

}