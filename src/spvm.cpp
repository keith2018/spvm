/*
 * spvm
 * @author 	: keith@robot9.me
 *
 */

#include "spvm.h"
#include "image.h"
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

}