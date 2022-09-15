/*
 * spvm
 * @author 	: keith@robot9.me
 *
 */

#pragma once

#include <vector>
#include <unordered_map>
#include <string>

#include "spirv.h"

namespace SPVM {

#define EXT_GLSLstd450 "GLSL.std.450"

#define VECTOR_boolean(idx) value.members[(idx)]->value.boolean
#define VECTOR_i32(idx)  value.members[(idx)]->value.i32
#define VECTOR_u32(idx)  value.members[(idx)]->value.u32
#define VECTOR_f32(idx)  value.members[(idx)]->value.f32
#define VECTOR_value(idx)  value.members[(idx)]->value

#define MATRIX_boolean(col, row) value.members[(col)]->value.members[(row)]->value.boolean
#define MATRIX_i32(col, row)  value.members[(col)]->value.members[(row)]->value.i32
#define MATRIX_u32(col, row)  value.members[(col)]->value.members[(row)]->value.u32
#define MATRIX_f32(col, row)  value.members[(col)]->value.members[(row)]->value.f32
#define MATRIX_value(col, row)  value.members[(col)]->value.members[(row)]->value

typedef void      SpvmVoid;
typedef uint8_t   SpvmByte;
typedef uint32_t  SpvmWord;
typedef bool      SpvmBool;
typedef int8_t    SpvmI8;
typedef uint8_t   SpvmU8;
typedef int16_t   SpvmI16;
typedef uint16_t  SpvmU16;
typedef int32_t   SpvmI32;
typedef uint32_t  SpvmU32;
typedef int64_t   SpvmI64;
typedef uint64_t  SpvmU64;
typedef float     SpvmF32;
typedef double    SpvmF64;

#define SpvmTrue  true
#define SpvmFalse false
#define SpvmResultIdInvalid 0

typedef struct SpvmString_ {
  SpvmWord wordCount;
  SpvmWord strLen;
  SpvmByte *str;
} SpvmString;

typedef struct SpvmOpcode_ {
  SpvOp op;
  SpvmWord wordCount;
} SpvmOpcode;

typedef struct SpvmDebugSource_ {
  SpvSourceLanguage language;
  SpvmWord languageVersion;
} SpvmSource;

typedef struct SpvmName_ {
  SpvmWord targetId;
  SpvmWord memberIdx;
  SpvmString name;
} SpvmName;

typedef struct SpvmExtension_ {
  SpvmString name;
} SpvmExtension;

typedef struct SpvmExtInstImport_ {
  SpvmWord setId;
  SpvmWord insCnt;
  void *insPtr[0];
} SpvmExtInstImport;

typedef struct SpvmEntryPoint_ {
  SpvExecutionModel executionModel;
  SpvmWord id;
  SpvmString name;
  SpvmWord globalIdCnt;
  SpvmWord *globalIds;
} SpvmEntryPoint;

typedef struct SpvmExecutionMode_ {
  SpvmWord entryPointId;
  SpvExecutionMode mode;
} SpvmExecutionMode;

typedef struct SpvmDecorationExtra_ {
  union {
    SpvmWord location;
    SpvBuiltIn builtIn;
    SpvmWord binding;
    SpvmWord descriptorSet;
  } value;
} SpvmDecorationExtra;

typedef struct SpvmDecoration_ {
  SpvmWord targetId;
  SpvmWord memberIdx;
  SpvDecoration decoration;
  SpvmDecorationExtra extra;
} SpvmDecoration;

typedef struct SpvmTypeBase_ {
  SpvOp type;
  SpvmWord memberCnt;
  SpvmWord resultId;
} SpvmTypeBase;

typedef struct SpvmTypeVoid_ : SpvmTypeBase {
} SpvmTypeVoid;

typedef struct SpvmTypeBool_ : SpvmTypeBase {
} SpvmTypeBool;

typedef struct SpvmTypeInt_ : SpvmTypeBase {
  SpvmWord width;
  SpvmByte signedness;
} SpvmTypeInt;

typedef struct SpvmTypeFloat_ : SpvmTypeBase {
  SpvmWord width;
} SpvmTypeFloat;

typedef struct SpvmTypeVector_ : SpvmTypeBase {
  SpvmTypeBase *componentType;
} SpvmTypeVector;

typedef struct SpvmTypeMatrix_ : SpvmTypeBase {
  SpvmTypeBase *columnType;
} SpvmTypeMatrix;

typedef struct SpvmTypeImage_ : SpvmTypeBase {
  SpvmTypeBase *sampledType;
  SpvDim dim;
  SpvmByte depth;
  SpvmByte arrayed;
  SpvmByte ms;
  SpvmByte sampled;
  SpvImageFormat format;
  SpvAccessQualifier accessQualifier;
} SpvmTypeImage;

typedef struct SpvmTypeSampler_ : SpvmTypeBase {
} SpvmTypeSampler;

typedef struct SpvmTypeSampledImage_ : SpvmTypeBase {
  SpvmTypeImage *imageType;
} SpvmTypeSampledImage;

typedef struct SpvmTypeArray_ : SpvmTypeBase {
  SpvmTypeBase *elementType;
} SpvmTypeArray;

typedef struct SpvmTypeRuntimeArray_ : SpvmTypeBase {
  SpvmTypeBase *elementType;
} SpvmTypeRuntimeArray;

typedef struct SpvmTypeStruct_ : SpvmTypeBase {
  SpvmTypeBase **memberTypes;
} SpvmTypeStruct;

typedef struct SpvmTypePointer_ : SpvmTypeBase {
  SpvStorageClass storageClass;
  SpvmTypeBase *objType;
} SpvmTypePointer;

typedef struct SpvmTypeFunction_ : SpvmTypeBase {
  SpvmTypeBase *returnType;
  SpvmWord parameterCnt;
  SpvmTypeBase **parameterTypes;
} SpvmTypeFunction;

typedef struct SpvmFunction_ {
  SpvmWord functionControl;
  SpvmTypeBase *functionType;
  SpvmWord *code;
} SpvmFunction;

typedef struct SpvmSamplerAttribute_ {
  SpvSamplerAddressingMode addressingMode;
  SpvmByte normalized;
  SpvSamplerFilterMode filterMode;
} SpvmSamplerAttribute;

typedef struct SpvmSamplerInternal_ {
  SpvmSamplerAttribute *attributes;
  SpvmVoid *opaque;
} SpvmSamplerInternal;

typedef struct SpvmImageInternal_ {
  SpvmTypeImage *type;
  SpvmVoid *opaque;
} SpvmImageInternal;

typedef struct SpvmSampledImageInternal_ {
  SpvmImageInternal *image;
  SpvmSamplerInternal *sampler;
} SpvmSampledImageInternal;

typedef struct SpvmValue_ {
  SpvmTypeBase *type;
  SpvmWord memberCnt;

  union {
    SpvmBool boolean;
    SpvmI32 i32;
    SpvmU32 u32;
    SpvmF32 f32;

    SpvmSamplerInternal *sampler;
    SpvmImageInternal *image;
    SpvmSampledImageInternal *sampledImage;

    SpvmValue_ **members;
  } value;
} SpvmValue;

typedef struct SpvmPointer_ {
  SpvmTypePointer *resultType;
  SpvmValue *objPtr;
} SpvmPointer;

typedef struct SpvmImageOperands_ {
  SpvmValue *bias;
  SpvmValue *lod;
  SpvmValue *dx, *dy;
  SpvmValue *offset;
  SpvmValue *offsets; // OpImageGather, OpImageDrefGather
  SpvmValue *sample;
  SpvmValue *minLod;
} SpvmImageOperands;

typedef union SpvmVecElement_ {
  SpvmI32 i32;
  SpvmU32 u32;
  SpvmF32 f32;
} SpvmVecElement;

typedef struct SpvmVec2_ {
  SpvmVecElement elem[2];
} SpvmVec2;

typedef struct SpvmVec3_ {
  SpvmVecElement elem[3];
} SpvmVec3;

typedef struct SpvmVec4_ {
  SpvmVecElement elem[4];
} SpvmVec4;

SpvmValue *createValue(SpvmTypeBase *type, SpvmByte **psp);
void copyValue(SpvmValue *dst, SpvmValue *src);
SpvmWord getTypeSize(SpvmTypeBase *type);
void writeToValue(SpvmValue *retValue, SpvmVec4 vec);
SpvmVec4 readFromValue(SpvmValue *value);

void valueSubF32(SpvmValue *ret, SpvmValue *a, SpvmValue *b);

SpvmVec4 getDPdx(void *ctx, SpvmWord P);
SpvmVec4 getDPdy(void *ctx, SpvmWord P);

}