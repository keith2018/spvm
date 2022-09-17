/*
 * spvm
 * @author 	: keith@robot9.me
 *
 */

#include "spvm.h"
#include "quad.h"
#include "opstrings.h"
#include "logger.h"

namespace SPVM {

#ifndef HEAP_MALLOC
#define HEAP_MALLOC(n) sp; sp += (n)
#endif

SpvmValue *createValue(SpvmTypeBase *type, SpvmByte **psp) {
  if (type->type == SpvOpTypeVoid) {
    return nullptr;
  }
  SpvmByte *sp = *psp;
  SpvmValue *ret = (SpvmValue *) HEAP_MALLOC(sizeof(SpvmValue));
  ret->type = type;
  ret->memberCnt = type->memberCnt;
  switch (type->type) {
    case SpvOpTypeBool:
    case SpvOpTypeInt:
    case SpvOpTypeFloat: {
      ret->value.members = nullptr;
      break;
    }
    case SpvOpTypeVector: {
      SpvmTypeVector *vecType = (SpvmTypeVector *) type;
      ret->value.members = (SpvmValue **) HEAP_MALLOC(ret->memberCnt * sizeof(SpvmValue *));
      for (SpvmWord i = 0; i < ret->memberCnt; i++) {
        ret->value.members[i] = createValue(vecType->componentType, &sp);
      }
      break;
    }
    case SpvOpTypeMatrix: {
      SpvmTypeMatrix *matType = (SpvmTypeMatrix *) type;
      ret->value.members = (SpvmValue **) HEAP_MALLOC(ret->memberCnt * sizeof(SpvmValue *));
      for (SpvmWord i = 0; i < ret->memberCnt; i++) {
        ret->value.members[i] = createValue(matType->columnType, &sp);
      }
      break;
    }
    case SpvOpTypeArray: {
      SpvmTypeArray *arrayType = (SpvmTypeArray *) type;
      ret->value.members = (SpvmValue **) HEAP_MALLOC(ret->memberCnt * sizeof(SpvmValue *));
      for (SpvmWord i = 0; i < ret->memberCnt; i++) {
        ret->value.members[i] = createValue(arrayType->elementType, &sp);
      }
      break;
    }
    case SpvOpTypeRuntimeArray: {
      SpvmTypeRuntimeArray *rtArrType = (SpvmTypeRuntimeArray *) type;
      if (rtArrType->memberCnt <= 0) {
        ret->value.members = nullptr;
      } else {
        ret->value.members = (SpvmValue **) HEAP_MALLOC(ret->memberCnt * sizeof(SpvmValue *));
        for (SpvmWord i = 0; i < ret->memberCnt; i++) {
          ret->value.members[i] = createValue(rtArrType->elementType, &sp);
        }
      }
      break;
    }
    case SpvOpTypeStruct: {
      SpvmTypeStruct *structType = (SpvmTypeStruct *) type;
      ret->value.members = (SpvmValue **) HEAP_MALLOC(ret->memberCnt * sizeof(SpvmValue *));
      for (SpvmWord i = 0; i < ret->memberCnt; i++) {
        ret->value.members[i] = createValue(structType->memberTypes[i], &sp);
      }
      break;
    }
    case SpvOpTypeImage: {
      SpvmTypeImage *imgType = (SpvmTypeImage *) type;
      ret->value.image = (SpvmImageInternal *) HEAP_MALLOC(sizeof(SpvmImageInternal));
      ret->value.image->type = imgType;
      ret->value.image->opaque = nullptr;
      break;
    }
    case SpvOpTypeSampler: {
      ret->value.sampler = (SpvmSamplerInternal *) HEAP_MALLOC(sizeof(SpvmSamplerInternal));
      ret->value.sampler->attributes = nullptr;
      ret->value.sampler->opaque = nullptr;
      break;
    }
    case SpvOpTypeSampledImage: {
      SpvmTypeSampledImage *sampledImgType = (SpvmTypeSampledImage *) type;
      ret->value.sampledImage = (SpvmSampledImageInternal *) HEAP_MALLOC(sizeof(SpvmSampledImageInternal));
      SpvmValue *imageVal = createValue(sampledImgType->imageType, &sp);
      ret->value.sampledImage->image = imageVal->value.image;
      ret->value.sampledImage->sampler = (SpvmSamplerInternal *) HEAP_MALLOC(sizeof(SpvmSamplerInternal));
      ret->value.sampledImage->sampler->attributes = nullptr;
      ret->value.sampledImage->sampler->opaque = nullptr;
      break;
    }
    default: {
      LOGE("createWithType not implement: %s", spvmOpString(type->type));
      break;
    }
  }

  *psp = sp;
  return ret;
}

SpvmWord getTypeSize(SpvmTypeBase *type) {
  switch (type->type) {
    case SpvOpTypeBool:
      return sizeof(SpvmBool);
    case SpvOpTypeInt: {
      SpvmTypeInt *intType = (SpvmTypeInt *) type;
      return intType->width / 8;
    }
    case SpvOpTypeFloat: {
      SpvmTypeFloat *floatType = (SpvmTypeFloat *) type;
      return floatType->width / 8;
    }
    case SpvOpTypeRuntimeArray: {
      SpvmTypeRuntimeArray *rtArrType = (SpvmTypeRuntimeArray *) type;
      return rtArrType->memberCnt * getTypeSize(rtArrType->elementType);
    }
    case SpvOpTypeVector: {
      SpvmTypeVector *vecType = (SpvmTypeVector *) type;
      return vecType->memberCnt * getTypeSize(vecType->componentType);
    }
    case SpvOpTypeMatrix: {
      SpvmTypeMatrix *matType = (SpvmTypeMatrix *) type;
      return matType->memberCnt * getTypeSize(matType->columnType);
    }
    case SpvOpTypeArray: {
      SpvmTypeArray *arrType = (SpvmTypeArray *) type;
      return arrType->memberCnt * getTypeSize(arrType->elementType);
    }
    case SpvOpTypeStruct: {
      SpvmTypeStruct *structType = (SpvmTypeStruct *) type;
      SpvmWord totalSize = 0;
      for (SpvmWord i = 0; i < structType->memberCnt; i++) {
        totalSize += getTypeSize(structType->memberTypes[i]);
      }
      return totalSize;
    }
    default:
      break;
  }

  return 0;
}

void copyValue(SpvmValue *dst, SpvmValue *src) {
  if (src->memberCnt > 0) {
    for (SpvmWord i = 0; i < src->memberCnt; i++) {
      copyValue(dst->value.members[i], src->value.members[i]);
    }
  } else {
    dst->value = src->value;
  }
}

SpvmValue *setValueF32(SpvmValue *ret, SpvmF32 val) {
  if (ret->memberCnt > 0) {
    for (SpvmWord i = 0; i < ret->memberCnt; i++) {
      ret->VECTOR_f32(i) = val;
    }
  } else {
    ret->value.f32 = val;
  }

  return ret;
}

void writeToValue(SpvmValue *retValue, SpvmVec4 vec) {
  if (retValue->memberCnt == 0) {
    retValue->value.i32 = vec.elem[0].i32;
  } else {
    for (SpvmWord i = 0; i < retValue->memberCnt && i < 4; i++) {
      retValue->value.members[i]->value.i32 = vec.elem[i].i32;
    }
  }
}

SpvmVec4 readFromValue(SpvmValue *value) {
  SpvmVec4 retVec{0, 0, 0, 0};
  if (value->memberCnt > 0) {
    for (SpvmWord i = 0; i < value->memberCnt && i < 3; i++) {
      retVec.elem[i].i32 = value->value.members[i]->value.i32;
    }
  } else {
    retVec.elem[0].i32 = value->value.i32;
  }
  return retVec;
}

void valueSubF32(SpvmValue *ret, SpvmValue *a, SpvmValue *b) {
  if (ret->memberCnt > 0) {
    for (SpvmWord i = 0; i < ret->memberCnt; i++) {
      ret->VECTOR_f32(i) = a->VECTOR_f32(i) - b->VECTOR_f32(i);
    }
  } else {
    ret->value.f32 = a->value.f32 - b->value.f32;
  }
}

SpvmVec4 getDPdx(void *ctx, SpvmWord P) {
  SpvmVec4 ret{0, 0, 0, 0};
  RuntimeContext *rtCtx = (RuntimeContext *) ctx;
  if (rtCtx->quadCtx == nullptr) {
//    LOGE("getDPdx error: no quad context");
    return ret;
  }
  SpvmValue *dx = rtCtx->quadCtx->getDPdx(rtCtx->quadIdx, P);
  if (dx) {
    ret = readFromValue(dx);
  }
  return ret;
}

SpvmVec4 getDPdy(void *ctx, SpvmWord P) {
  SpvmVec4 ret{0, 0, 0, 0};
  RuntimeContext *rtCtx = (RuntimeContext *) ctx;
  if (rtCtx->quadCtx == nullptr) {
//    LOGE("getDPdx error: no quad context");
    return ret;
  }
  SpvmValue *dy = rtCtx->quadCtx->getDPdy(rtCtx->quadIdx, P);
  if (dy) {
    ret = readFromValue(dy);
  }
  return ret;
}

}