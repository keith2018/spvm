/*
 * spvm
 * @author 	: keith@robot9.me
 *
 */

#include "interface.h"
#include "runtime.h"
#include "module.h"
#include "image.h"
#include "logger.h"

namespace SPVM {

void Interface::init(RuntimeContext *ctx, void **resultIds) {
  runtimeCtx_ = ctx;
  resultIds_ = resultIds;
  SpvmModule *module = ctx->module;

  // init uniform location & bindings
  for (auto &pointerId : module->globalPointers) {
    SpvmPointer *pointer = (SpvmPointer *) resultIds_[pointerId];
    switch (pointer->resultType->storageClass) {
      case SpvStorageClassUniformConstant:
      case SpvStorageClassInput:
      case SpvStorageClassUniform:
      case SpvStorageClassOutput: {
        auto it = module->decorations.find(pointerId);
        if (it != module->decorations.end()) {
          checkDecorations(pointer, pointerId, it->second);
        }

        SpvmWord typeId = pointer->resultType->objType->resultId;
        it = module->decorations.find(typeId);
        if (it != module->decorations.end()) {
          checkDecorations(pointer, pointerId, it->second);
        }
        break;
      }
      case SpvStorageClassPushConstant: {
        pushConstants_ = pointer->objPtr;
        break;
      }
      default:
        break;
    }
  }
}

SpvmWord Interface::getLocationByName(const char *name) {
  SpvmModule *module = runtimeCtx_->module;
  for (SpvmWord i = 0; i < module->names.size(); i++) {
    SpvmName *nameObj = &module->names[i];
    if (strcmp((const char *) nameObj->name.str, name) == 0 && nameObj->memberIdx == -1) {
      return locationMap_[nameObj->targetId];
    }
  }
  LOGE("getLocationByName not found: %s", name);
  return (SpvmWord) -1;
}

SpvmUniformBinding Interface::getBindingByName(const char *name) {
  SpvmModule *module = runtimeCtx_->module;
  for (SpvmWord i = 0; i < module->names.size(); i++) {
    SpvmName *nameObj = &module->names[i];
    if (strcmp((const char *) nameObj->name.str, name) == 0 && nameObj->memberIdx == -1) {
      return bindingMap_[nameObj->targetId];
    }
  }
  LOGE("getBindingByName not found: %s", name);
  return {(SpvmWord) -1, (SpvmWord) -1};
}

void Interface::writeInput(void *data, SpvmWord location) {
  auto it = inputLocation_.find(location);
  if (it == inputLocation_.end()) {
    LOGE("writeInput error, location not found: %d", location);
    return;
  }

  writeValue(it->second, data, 0);
}

void Interface::writeInputBuiltIn(void *data, SpvBuiltIn builtIn) {
  auto it = inoutBuiltIn_.find(builtIn);
  if (it == inoutBuiltIn_.end()) {
    LOGW("writeInputBuiltIn ignored, builtIn not used: %d", builtIn);
    return;
  }

  writeValue(it->second, data, 0);
}

void Interface::writeUniformBinding(void *data, SpvmWord length, SpvmWord binding, SpvmWord set) {
  SpvmUniformBinding location = {(SpvmWord) set, (SpvmWord) binding};
  auto it = bindings_.find(location);
  if (it == bindings_.end()) {
    LOGE("writeUniformBlock error, not found, binding: %d, set: %d", binding, set);
    return;
  }

  writeValue(it->second, data, length);
}

void Interface::writeUniformPushConstants(void *data, SpvmWord length) {
  if (pushConstants_ == nullptr) {
    LOGE("writeUniformPushConstants error, PushConstants not found");
    return;
  }

  writeValue(pushConstants_, data, length);
}

void Interface::readOutput(void *data, SpvmWord location) {
  auto it = outputLocation_.find(location);
  if (it == outputLocation_.end()) {
    LOGE("readOutput error, location not found: %d", location);
    return;
  }

  readValue(it->second, data);
}

void Interface::readOutputBuiltIn(void *data, SpvBuiltIn builtIn) {
  auto it = inoutBuiltIn_.find(builtIn);
  if (it == inoutBuiltIn_.end()) {
    LOGE("readOutputBuiltIn error, builtIn not found: %d", builtIn);
    return;
  }

  readValue(it->second, data);
}

void Interface::checkDecorations(SpvmPointer *pointer, SpvmWord pointerId, std::vector<SpvmDecoration> &decorations) {
  int binding = -1;
  int set = 0;
  for (auto &dec : decorations) {
    switch (dec.decoration) {
      case SpvDecorationLocation: {
        if (pointer->resultType->storageClass == SpvStorageClassInput) {
          inputLocation_[dec.extra.value.location] = pointer->objPtr;
        } else if (pointer->resultType->storageClass == SpvStorageClassOutput) {
          outputLocation_[dec.extra.value.location] = pointer->objPtr;
        }
        locationMap_[pointerId] = dec.extra.value.location;
        break;
      }
      case SpvDecorationBuiltIn: {
        SpvmValue *valuePtr = pointer->objPtr;
        if (dec.memberIdx != -1) {
          valuePtr = pointer->objPtr->value.members[dec.memberIdx];
        }
        inoutBuiltIn_[dec.extra.value.builtIn] = valuePtr;
        break;
      }
      case SpvDecorationBinding:
        binding = (int) dec.extra.value.binding;
        break;
      case SpvDecorationDescriptorSet:
        set = (int) dec.extra.value.descriptorSet;
        break;
      default:
        break;
    }
  }

  if (binding != -1) {
    SpvmUniformBinding location = {(SpvmWord) set, (SpvmWord) binding};
    bindings_[location] = pointer->objPtr;
    bindingMap_[pointerId] = location;
  }
}

void *Interface::writeValue(SpvmValue *value, void *data, SpvmWord length) {
  switch (value->type->type) {
    case SpvOpTypeBool: {
      SpvmBool *ptr = (SpvmBool *) data;
      value->value.boolean = ptr ? *ptr : SpvmFalse;
      return ++ptr;
    }
    case SpvOpTypeInt: {
      SpvmTypeInt *intType = (SpvmTypeInt *) value->type;
      if (intType->width == 32) {
        if (intType->signedness) {
          SpvmI32 *ptr = (SpvmI32 *) data;
          value->value.i32 = ptr ? *ptr : 0;
          return ++ptr;
        } else {
          SpvmU32 *ptr = (SpvmU32 *) data;
          value->value.u32 = ptr ? *ptr : 0;
          return ++ptr;
        }
      }
      LOGE("writeValue SpvOpTypeInt error: bit width not support: %d", intType->width);
      return data;
    }
    case SpvOpTypeFloat: {
      SpvmTypeFloat *floatType = (SpvmTypeFloat *) value->type;
      if (floatType->width == 32) {
        SpvmF32 *ptr = (SpvmF32 *) data;
        value->value.f32 = ptr ? *ptr : 0;
        return ++ptr;
      }
      LOGE("writeValue SpvOpTypeFloat error: bit width not support: %d", floatType->width);
      return data;
    }
    case SpvOpTypeRuntimeArray: {
      SpvmTypeRuntimeArray *rtArrType = (SpvmTypeRuntimeArray *) value->type;

      // calculate element size
      SpvmWord elemSize = getTypeSize(rtArrType->elementType);
      rtArrType->memberCnt = length / elemSize;

      // create SpvmValue
#define HEAP_MALLOC(n) sp; sp += (n)
      SpvmByte *sp = runtimeCtx_->stackBase;
      value->memberCnt = rtArrType->memberCnt;
      value->value.members = (SpvmValue **) HEAP_MALLOC(value->memberCnt * sizeof(SpvmValue *));
      for (SpvmWord i = 0; i < value->memberCnt; i++) {
        value->value.members[i] = createValue(rtArrType->elementType, &sp);
      }
      runtimeCtx_->stackBase = sp;

      // write data
      void *ptr = data;
      for (int i = 0; i < value->memberCnt; i++) {
        ptr = writeValue(value->value.members[i], ptr, 0);
      }
      return ptr;
    }
    case SpvOpTypeVector:
    case SpvOpTypeMatrix:
    case SpvOpTypeArray:
    case SpvOpTypeStruct: {
      void *ptr = data;
      for (int i = 0; i < value->memberCnt; i++) {
        void *ptrLeft = writeValue(value->value.members[i], ptr, length);
        if (length > 0) {
          length -= ((SpvmByte *) ptrLeft - (SpvmByte *) data);
        }
        ptr = ptrLeft;
      }
      return ptr;
    }
    case SpvOpTypeSampledImage: {
      SpvmTypeSampledImage *sampledImageType = (SpvmTypeSampledImage *) value->type;
      SpvmSampledImage *ptr = (SpvmSampledImage *) data;
      bool checkOk = checkImageType(ptr->image, sampledImageType->imageType);
      if (checkOk) {
        value->value.sampledImage->image->opaque = ptr->image;
        value->value.sampledImage->sampler->opaque = ptr->sampler;
      } else {
        LOGE("write image failed: image info not match");
      }
      return ++ptr;
    }
    case SpvOpTypeImage: {
      SpvmTypeImage *imageType = (SpvmTypeImage *) value->type;
      SpvmImage *ptr = (SpvmImage *) data;
      bool checkOk = checkImageType(ptr, imageType);
      if (checkOk) {
        value->value.image->opaque = ptr;
      } else {
        LOGE("write image failed: image info not match");
      }
      return ++ptr;
    }
    case SpvOpTypeSampler: {
      SpvmSampler *ptr = (SpvmSampler *) data;
      value->value.sampler->opaque = ptr;
      return ++ptr;
    }
    default:
      break;
  }
  return data;
}

void *Interface::readValue(SpvmValue *value, void *data) {
  if (!value || !data) {
    return data;
  }

  switch (value->type->type) {
    case SpvOpTypeBool: {
      SpvmBool *ptr = (SpvmBool *) data;
      *ptr = value->value.boolean;
      return ++ptr;
    }
    case SpvOpTypeInt: {
      SpvmTypeInt *intType = (SpvmTypeInt *) value->type;
      if (intType->width == 32) {
        if (intType->signedness) {
          SpvmI32 *ptr = (SpvmI32 *) data;
          *ptr = (SpvmI32) value->value.i32;
          return ++ptr;
        } else {
          SpvmU32 *ptr = (SpvmU32 *) data;
          *ptr = (SpvmU32) value->value.u32;
          return ++ptr;
        }
      }
      LOGE("writeValue SpvOpTypeInt error: bit width not support: %d", intType->width);
      return data;
    }
    case SpvOpTypeFloat: {
      SpvmTypeFloat *floatType = (SpvmTypeFloat *) value->type;
      if (floatType->width == 32) {
        SpvmF32 *ptr = (SpvmF32 *) data;
        *ptr = (SpvmF32) value->value.f32;
        return ++ptr;
      }
      LOGE("writeValue SpvOpTypeFloat error: bit width not support: %d", floatType->width);
      return data;
    }
    case SpvOpTypeVector:
    case SpvOpTypeMatrix:
    case SpvOpTypeArray:
    case SpvOpTypeRuntimeArray:
    case SpvOpTypeStruct: {
      void *ptr = data;
      for (int i = 0; i < value->memberCnt; i++) {
        ptr = readValue(value->value.members[i], ptr);
      }
      return ptr;
    }
    default:
      break;
  }
  return data;
}

}