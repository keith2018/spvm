/*
 * spvm
 * @author 	: keith@robot9.me
 *
 */

#include <cmath>

#include "spvm.h"
#include "quad.h"
#include "logger.h"
#include "utils.h"
#include "opstrings.h"
#include "image.h"

namespace SPVM {

typedef RuntimeResult (*OpFunction)(SpvmWord *, SpvmByte *, SpvmOpcode, RuntimeContext *);

static constexpr SpvmWord __opFuncCnt = 512;
static OpFunction __opFuncs[__opFuncCnt];

#define DEF_OP_FUNC(op) static RuntimeResult func_##op(SpvmWord *pc, SpvmByte *sp, SpvmOpcode opcode, RuntimeContext *ctx)
#define REG_OP_FUNC(op) __opFuncs[op] = func_##op;

#ifdef SPVM_OP_DISPATCH_TAIL_CALL
#define GO_NEXT                                                                     \
  if (ctx->untilResult != SpvmResultIdInvalid && ctx->results[ctx->untilResult]) {  \
    ctx->pc = pc;                                                                   \
    ctx->sp = sp;                                                                   \
    return Result_NoError;                                                          \
  }                                                                                 \
  opcode = READ_OPCODE();                                                           \
  return __opFuncs[opcode.op](pc, sp, opcode, ctx);
#else
#define GO_NEXT                                                                     \
  ctx->pc = pc;                                                                     \
  ctx->sp = sp;                                                                     \
  return Result_NoError;
#endif

#define READ_WORD() *pc++
#define SKIP_WORD() pc++
#define SKIP_WORD_N(count) pc += (count)
#define SKIP_CURRENT_OPCODE() SKIP_WORD_N(opcode.wordCount - 1)
#define REWIND_WORD_N(count) pc -= (count)
#define READ_STR() readString(&pc, &sp)
#define READ_OPCODE() readOpcode(pc++)
#define READ_OPERAND() (SpvmValue *) ctx->results[READ_WORD()]

#define HEAP_MALLOC(n) sp; sp += (n)
#define CREATE_VALUE(type) createValue((type), &sp)
#define COPY_VALUE(dst, src) copyValue((dst), (src))
#define SPVM_TYPE(typeId) ((SpvmTypeBase *) ctx->results[(typeId)])

static SpvmString readString(SpvmWord **ppc, SpvmByte **psp) {
  SpvmByte *sp = *psp;
  SpvmString ret;
  ret.strLen = strlen((const char *) *ppc);
  ret.wordCount = (ret.strLen + sizeof(SpvmWord)) / sizeof(SpvmWord);
  ret.str = HEAP_MALLOC(ret.strLen + 1);
  memcpy(&ret.str[0], *ppc, ret.strLen);
  ret.str[ret.strLen] = '\0';
  *ppc += ret.wordCount;
  *psp = sp;
  return ret;
}

static SpvmOpcode readOpcode(SpvmWord *pc) {
  SpvmWord word = READ_WORD();

  SpvmOpcode opcode;
  opcode.op = SpvOp(word & SpvOpCodeMask);
  opcode.wordCount = (word & (~SpvOpCodeMask)) >> SpvWordCountShift;

//  LOGD("read opcode: %s, size: %d", spvmOpString(opcode.op), opcode.wordCount);
  return opcode;
}

static SpvmDecorationExtra readDecorationExtra(SpvmWord **ppc, SpvDecoration decoration, SpvmWord wordCnt) {
  SpvmWord *pc = *ppc;
  SpvmDecorationExtra extra;
  switch (decoration) {
    case SpvDecorationBuiltIn:
      extra.value.builtIn = (SpvBuiltIn) READ_WORD();
      break;
    case SpvDecorationLocation:
      extra.value.location = READ_WORD();
      break;
    case SpvDecorationBinding:
      extra.value.binding = READ_WORD();
      break;
    case SpvDecorationDescriptorSet:
      extra.value.descriptorSet = READ_WORD();
      break;
    default:
      SKIP_WORD_N(wordCnt);
      break;
  }

  *ppc = pc;
  return extra;
}

static SpvmImageOperands readImageOperands(SpvmWord **ppc, RuntimeContext *ctx, SpvmWord wordCnt) {
  SpvmImageOperands operands;
  memset(&operands, 0, sizeof(SpvmImageOperands));
  if (wordCnt <= 0) {
    return operands;
  }

  SpvmWord *pc = *ppc;
  SpvmWord mask = READ_WORD();
  if (mask & SpvImageOperandsBiasMask) {
    operands.bias = READ_OPERAND();
  }
  if (mask & SpvImageOperandsLodMask) {
    operands.lod = READ_OPERAND();
  }
  if (mask & SpvImageOperandsGradMask) {
    operands.dx = READ_OPERAND();
    operands.dy = READ_OPERAND();
  }
  if (mask & SpvImageOperandsConstOffsetMask) {
    operands.offset = READ_OPERAND();
  }
  if (mask & SpvImageOperandsOffsetMask) {
    operands.offset = READ_OPERAND();
  }
  if (mask & SpvImageOperandsConstOffsetsMask) {
    operands.offsets = READ_OPERAND();
  }
  if (mask & SpvImageOperandsSampleMask) {
    operands.sample = READ_OPERAND();
  }
  if (mask & SpvImageOperandsMinLodMask) {
    operands.minLod = READ_OPERAND();
  }

  *ppc = pc;
  return operands;
}

#include "ext/GLSL.std.450.inc"

DEF_OP_FUNC(SpvOpNop) {
  GO_NEXT
}

DEF_OP_FUNC(SpvOpSource) {
  if (ctx->module->inited) {
    SKIP_CURRENT_OPCODE();
  } else {
    SpvmSource source;
    source.language = (SpvSourceLanguage) READ_WORD();
    source.languageVersion = READ_WORD();
    // skip file & Source
    SKIP_WORD_N(opcode.wordCount - 3);
    ctx->module->source = source;
  }
  GO_NEXT
}

DEF_OP_FUNC(SpvOpSourceExtension) {
  if (ctx->module->inited) {
    SKIP_CURRENT_OPCODE();
  } else {
    SpvmExtension ext;
    ext.name = READ_STR();
    ctx->module->sourceExtensions.emplace_back(ext);
  }
  GO_NEXT
}

DEF_OP_FUNC(SpvOpName) {
  if (ctx->module->inited) {
    SKIP_CURRENT_OPCODE();
  } else {
    SpvmName name;
    name.targetId = READ_WORD();
    name.memberIdx = -1;
    name.name = READ_STR();
    ctx->module->names.emplace_back(name);
  }
  GO_NEXT
}

DEF_OP_FUNC(SpvOpMemberName) {
  if (ctx->module->inited) {
    SKIP_CURRENT_OPCODE();
  } else {
    SpvmName name;
    name.targetId = READ_WORD();
    name.memberIdx = READ_WORD();
    name.name = READ_STR();
    ctx->module->names.emplace_back(name);
  }
  GO_NEXT
}

DEF_OP_FUNC(SpvOpDecorate) {
  if (ctx->module->inited) {
    SKIP_CURRENT_OPCODE();
  } else {
    SpvmDecoration decoration;
    decoration.targetId = READ_WORD();
    decoration.memberIdx = -1;
    decoration.decoration = (SpvDecoration) READ_WORD();
    decoration.extra = readDecorationExtra(&pc, decoration.decoration, opcode.wordCount - 3);
    ctx->module->decorations[decoration.targetId].emplace_back(decoration);
  }
  GO_NEXT
}

DEF_OP_FUNC(SpvOpMemberDecorate) {
  if (ctx->module->inited) {
    SKIP_CURRENT_OPCODE();
  } else {
    SpvmDecoration decoration;
    decoration.targetId = READ_WORD();
    decoration.memberIdx = READ_WORD();
    decoration.decoration = (SpvDecoration) READ_WORD();
    decoration.extra = readDecorationExtra(&pc, decoration.decoration, opcode.wordCount - 4);
    ctx->module->decorations[decoration.targetId].emplace_back(decoration);
  }
  GO_NEXT
}

DEF_OP_FUNC(SpvOpExtension) {
  if (ctx->module->inited) {
    SKIP_CURRENT_OPCODE();
  } else {
    SpvmExtension extension;
    extension.name = READ_STR();
    ctx->module->extensions.emplace_back(extension);
  }
  GO_NEXT
}

DEF_OP_FUNC(SpvOpExtInstImport) {
  SpvmWord resultId = READ_WORD();
  SpvmString extName = READ_STR();
  if (strcmp(EXT_GLSLstd450, (const char *) extName.str) == 0) {
    SpvmWord extStructSize = sizeof(SpvmExtInstImport) + GLSLstd450Count * sizeof(void *);
    SpvmExtInstImport *ext = (SpvmExtInstImport *) HEAP_MALLOC(extStructSize);
    ext->setId = resultId;
    ext->insCnt = GLSLstd450Count;
    registerGLSLstd450(&ext->insPtr[0], ext->insCnt);
    ctx->results[resultId] = ext;
  } else {
    LOGE("ExtInstImport not support: %s", extName.str);
    return Result_Error;
  }

  GO_NEXT
}

DEF_OP_FUNC(SpvOpExtInst) {
  SKIP_WORD_N(2);
  SpvmExtInstImport *ext = (SpvmExtInstImport *) ctx->results[READ_WORD()];
  OpFunction extFunc = (OpFunction) ext->insPtr[READ_WORD()];
  REWIND_WORD_N(4);
  return extFunc(pc, sp, opcode, ctx);
}

DEF_OP_FUNC(SpvOpMemoryModel) {
  if (ctx->module->inited) {
    SKIP_CURRENT_OPCODE();
  } else {
    ctx->module->addressingModel = (SpvAddressingModel) READ_WORD();
    ctx->module->memoryModel = (SpvMemoryModel) READ_WORD();
    if (ctx->module->addressingModel != SpvAddressingModelLogical) {
      LOGE("Addressing model not support: %d", ctx->module->addressingModel);
      return Result_Error;
    }
    if (ctx->module->memoryModel != SpvMemoryModelSimple && ctx->module->memoryModel != SpvMemoryModelGLSL450) {
      LOGE("memory model not support: %d", ctx->module->memoryModel);
      return Result_Error;
    }
  }
  GO_NEXT
}

DEF_OP_FUNC(SpvOpEntryPoint) {
  if (ctx->module->inited) {
    SKIP_CURRENT_OPCODE();
  } else {
    SpvmEntryPoint entryPoint;
    entryPoint.executionModel = (SpvExecutionModel) READ_WORD();
    entryPoint.id = READ_WORD();
    entryPoint.name = READ_STR();
    entryPoint.globalIdCnt = opcode.wordCount - entryPoint.name.wordCount - 3;
    entryPoint.globalIds = (SpvmWord *) HEAP_MALLOC(entryPoint.globalIdCnt * sizeof(SpvmWord));
    for (SpvmWord i = 0; i < entryPoint.globalIdCnt; i++) {
      entryPoint.globalIds[i] = READ_WORD();
    }
    ctx->module->entryPoints.emplace_back(entryPoint);
  }
  GO_NEXT
}

DEF_OP_FUNC(SpvOpExecutionMode) {
  if (ctx->module->inited) {
    SKIP_CURRENT_OPCODE();
  } else {
    SpvmExecutionMode executionMode;
    executionMode.entryPointId = READ_WORD();
    executionMode.mode = (SpvExecutionMode) READ_WORD();
    SKIP_WORD_N(opcode.wordCount - 3); // extra
    ctx->module->executionModes.emplace_back(executionMode);
  }
  GO_NEXT
}

DEF_OP_FUNC(SpvOpCapability) {
  if (ctx->module->inited) {
    SKIP_CURRENT_OPCODE();
  } else {
    ctx->module->capabilities.emplace_back((SpvCapability) READ_WORD());
  }
  GO_NEXT
}

#define PARSE_TYPE_BEGIN(t)                                         \
    auto *type = (SpvmType##t *) HEAP_MALLOC(sizeof(SpvmType##t));  \
    type->type = SpvOpType##t;                                      \
    type->memberCnt = 0;                                            \
    type->resultId = READ_WORD();                                   \

#define PARSE_TYPE_END                                              \
    ctx->results[type->resultId] = type;                            \

DEF_OP_FUNC(SpvOpTypeVoid) {
  PARSE_TYPE_BEGIN(Void)
  PARSE_TYPE_END
  GO_NEXT
}

DEF_OP_FUNC(SpvOpTypeBool) {
  PARSE_TYPE_BEGIN(Bool)
  PARSE_TYPE_END
  GO_NEXT
}

DEF_OP_FUNC(SpvOpTypeInt) {
  PARSE_TYPE_BEGIN(Int)
  SpvmWord width = READ_WORD();
  type->width = width;
  if (width != 32) {  // only support 32bit integer
    LOGE("SpvOpTypeInt bit width not support: %d", width);
    return Result_Error;
  }
  type->signedness = READ_WORD();
  PARSE_TYPE_END
  GO_NEXT
}

DEF_OP_FUNC(SpvOpTypeFloat) {
  PARSE_TYPE_BEGIN(Float)
  SpvmWord width = READ_WORD();
  type->width = width;
  if (width != 32) {  // only support 32bit float
    LOGE("SpvOpTypeFloat bit width not support: %d", width);
    return Result_Error;
  }
  PARSE_TYPE_END
  GO_NEXT
}

DEF_OP_FUNC(SpvOpTypeVector) {
  PARSE_TYPE_BEGIN(Vector)
  type->componentType = SPVM_TYPE(READ_WORD());
  type->memberCnt = READ_WORD();
  PARSE_TYPE_END
  GO_NEXT
}

DEF_OP_FUNC(SpvOpTypeMatrix) {
  PARSE_TYPE_BEGIN(Matrix)
  type->columnType = SPVM_TYPE(READ_WORD());
  type->memberCnt = READ_WORD();
  PARSE_TYPE_END
  GO_NEXT
}

DEF_OP_FUNC(SpvOpTypeImage) {
  PARSE_TYPE_BEGIN(Image)
  type->sampledType = SPVM_TYPE(READ_WORD());
  type->dim = (SpvDim) READ_WORD();
  type->depth = READ_WORD();
  type->arrayed = READ_WORD();
  type->ms = READ_WORD();
  type->sampled = READ_WORD();
  type->format = (SpvImageFormat) READ_WORD();
  if (opcode.wordCount > 9) {
    type->accessQualifier = (SpvAccessQualifier) READ_WORD();
  } else {
    type->accessQualifier = SpvAccessQualifierMax;
  }
  PARSE_TYPE_END
  GO_NEXT
}

DEF_OP_FUNC(SpvOpTypeSampler) {
  PARSE_TYPE_BEGIN(Sampler)
  PARSE_TYPE_END
  GO_NEXT
}

DEF_OP_FUNC(SpvOpTypeSampledImage) {
  PARSE_TYPE_BEGIN(SampledImage)
  type->imageType = (SpvmTypeImage *) SPVM_TYPE(READ_WORD());
  PARSE_TYPE_END
  GO_NEXT
}

DEF_OP_FUNC(SpvOpTypeArray) {
  PARSE_TYPE_BEGIN(Array)
  type->elementType = SPVM_TYPE(READ_WORD());
  SpvmValue *lengthVal = READ_OPERAND();
  type->memberCnt = lengthVal->value.u32;
  PARSE_TYPE_END
  GO_NEXT
}

DEF_OP_FUNC(SpvOpTypeRuntimeArray) {
  PARSE_TYPE_BEGIN(RuntimeArray)
  type->elementType = SPVM_TYPE(READ_WORD());
  type->memberCnt = 0;
  PARSE_TYPE_END
  GO_NEXT
}

DEF_OP_FUNC(SpvOpTypeStruct) {
  PARSE_TYPE_BEGIN(Struct)
  type->memberCnt = opcode.wordCount - 2;
  type->memberTypes = (SpvmTypeBase **) HEAP_MALLOC(type->memberCnt * sizeof(SpvmTypeBase *));
  for (SpvmWord i = 0; i < type->memberCnt; i++) {
    type->memberTypes[i] = SPVM_TYPE(READ_WORD());
  }
  PARSE_TYPE_END
  GO_NEXT
}

DEF_OP_FUNC(SpvOpTypePointer) {
  PARSE_TYPE_BEGIN(Pointer)
  type->storageClass = (SpvStorageClass) READ_WORD();
  type->objType = SPVM_TYPE(READ_WORD());
  PARSE_TYPE_END
  GO_NEXT
}

DEF_OP_FUNC(SpvOpTypeFunction) {
  PARSE_TYPE_BEGIN(Function)
  type->returnType = SPVM_TYPE(READ_WORD());
  type->parameterCnt = opcode.wordCount - 3;
  type->parameterTypes = (SpvmTypeBase **) HEAP_MALLOC(type->parameterCnt * sizeof(SpvmTypeBase *));
  for (SpvmWord i = 0; i < type->parameterCnt; i++) {
    type->parameterTypes[i] = SPVM_TYPE(READ_WORD());
  }
  PARSE_TYPE_END
  GO_NEXT
}

DEF_OP_FUNC(SpvOpConstantTrue) {
  SpvmTypeBase *resultType = SPVM_TYPE(READ_WORD());
  SpvmWord resultId = READ_WORD();
  SpvmValue *var = CREATE_VALUE(resultType);
  var->value.boolean = true;
  ctx->results[resultId] = var;
  GO_NEXT
}

DEF_OP_FUNC(SpvOpConstantFalse) {
  SpvmTypeBase *resultType = SPVM_TYPE(READ_WORD());
  SpvmWord resultId = READ_WORD();
  SpvmValue *var = CREATE_VALUE(resultType);
  var->value.boolean = false;
  ctx->results[resultId] = var;
  GO_NEXT
}

DEF_OP_FUNC(SpvOpConstant) {
  SpvmTypeBase *resultType = SPVM_TYPE(READ_WORD());
  SpvmWord resultId = READ_WORD();
  SpvmValue *var = CREATE_VALUE(resultType);

  SpvmWord numberWidth = opcode.wordCount - 3;
  if (numberWidth == 1) {
    var->value.u32 = READ_WORD();
  } else {
    LOGE("SpvOpConstant value bit width not support: %d", numberWidth);
    return Result_Error;
  }
  ctx->results[resultId] = var;
  GO_NEXT
}

DEF_OP_FUNC(SpvOpConstantComposite) {
  SpvmTypeBase *resultType = SPVM_TYPE(READ_WORD());
  SpvmWord resultId = READ_WORD();
  SpvmValue *var = CREATE_VALUE(resultType);
  SpvmWord idxCnt = opcode.wordCount - 3;
  for (SpvmWord i = 0; i < idxCnt; i++) {
    var->value.members[i] = READ_OPERAND();
  }
  ctx->results[resultId] = var;
  GO_NEXT
}

DEF_OP_FUNC(SpvOpConstantSampler) {
  SpvmTypeBase *resultType = SPVM_TYPE(READ_WORD());
  SpvmWord resultId = READ_WORD();
  SpvmValue *var = CREATE_VALUE(resultType);
  SpvmSamplerAttribute *attributes = (SpvmSamplerAttribute *) HEAP_MALLOC(sizeof(SpvmSamplerAttribute));
  attributes->addressingMode = (SpvSamplerAddressingMode) READ_WORD();
  attributes->normalized = READ_WORD();
  attributes->filterMode = (SpvSamplerFilterMode) READ_WORD();
  var->value.sampler->attributes = attributes;
  // create sampler opaque with attributes
  var->value.sampler->opaque = createSamplerConstant(attributes);
  ctx->results[resultId] = var;
  GO_NEXT
}

DEF_OP_FUNC(SpvOpConstantNull) {
  SpvmTypeBase *resultType = SPVM_TYPE(READ_WORD());
  SpvmWord resultId = READ_WORD();
  SpvmValue *var = CREATE_VALUE(resultType);
  ctx->results[resultId] = var;
  GO_NEXT
}

DEF_OP_FUNC(SpvOpVariable) {
  SpvmPointer *pointer = (SpvmPointer *) HEAP_MALLOC(sizeof(SpvmPointer));
  pointer->resultType = (SpvmTypePointer *) SPVM_TYPE(READ_WORD());
  SpvmWord resultId = READ_WORD();
  SKIP_WORD();  // storageClass
  if (opcode.wordCount > 4) {
    SpvmWord initializerId = READ_WORD();
    LOGE("OpVariable initializer not implement");
  }

  // global variables
  if (!ctx->module->inited) {
    ctx->module->globalPointers.emplace_back(resultId);
  }

  pointer->objPtr = CREATE_VALUE(pointer->resultType->objType);
  ctx->results[resultId] = pointer;

  GO_NEXT
}

DEF_OP_FUNC(SpvOpLoad) {
  SKIP_WORD();
  SpvmWord resultId = READ_WORD();
  SpvmWord pointerId = READ_WORD();
  if (opcode.wordCount > 4) {
    SpvmWord memoryOperands = READ_WORD();
    LOGE("OpLoad memoryOperands not implement");
  }

  SpvmPointer *pointer = (SpvmPointer *) ctx->results[pointerId];
  ctx->results[resultId] = pointer->objPtr;
  GO_NEXT
}

DEF_OP_FUNC(SpvOpStore) {
  SpvmWord pointerId = READ_WORD();
  SpvmWord objId = READ_WORD();
  if (opcode.wordCount > 3) {
    SpvmWord memoryOperands = READ_WORD();
    LOGE("OpStore memoryOperands not implement");
  }

  SpvmPointer *pointer = (SpvmPointer *) ctx->results[pointerId];
  SpvmValue *obj = (SpvmValue *) ctx->results[objId];
  COPY_VALUE(pointer->objPtr, obj);
  GO_NEXT
}

DEF_OP_FUNC(SpvOpAccessChain) {
  SpvmPointer *pointer = (SpvmPointer *) HEAP_MALLOC(sizeof(SpvmPointer));
  pointer->resultType = (SpvmTypePointer *) SPVM_TYPE(READ_WORD());
  SpvmWord resultId = READ_WORD();

  SpvmWord baseId = READ_WORD();
  SpvmWord idxCnt = opcode.wordCount - 4;
  SpvmPointer *base = (SpvmPointer *) ctx->results[baseId];
  SpvmValue *valuePtr = base->objPtr;
  for (SpvmWord i = 0; i < idxCnt; i++) {
    SpvmValue *memberIdx = READ_OPERAND();
    if (memberIdx->value.u32 > valuePtr->memberCnt) {
      LOGE("OpAccessChain index out of bounds: %d > %d", memberIdx->value.u32, valuePtr->memberCnt);
      return Result_Error;
    }
    valuePtr = valuePtr->value.members[memberIdx->value.u32];
  }

  pointer->objPtr = valuePtr;
  ctx->results[resultId] = pointer;
  GO_NEXT
}

DEF_OP_FUNC(SpvOpArrayLength) {
  SpvmTypeBase *resultType = SPVM_TYPE(READ_WORD());
  SpvmWord resultId = READ_WORD();
  SpvmPointer *ptr = (SpvmPointer *) ctx->results[READ_WORD()];
  SpvmValue *runtimeArr = ptr->objPtr->value.members[READ_WORD()];
  SpvmValue *ret = CREATE_VALUE(resultType);
  ret->value.u32 = runtimeArr->memberCnt;
  ctx->results[resultId] = ret;
  GO_NEXT
}

DEF_OP_FUNC(SpvOpFunction) {
  SpvmFunction *func = (SpvmFunction *) HEAP_MALLOC(sizeof(SpvmFunction));
  SKIP_WORD(); // result type
  SpvmWord resultId = READ_WORD();
  func->functionControl = READ_WORD();
  func->functionType = SPVM_TYPE(READ_WORD());
  func->code = pc;
  ctx->results[resultId] = func;

  // skip function codes
  SpvmOpcode insOp = READ_OPCODE();
  while (insOp.op != SpvOpFunctionEnd) {
    // OpLabel
    if (insOp.op == SpvOpLabel) {
      SpvmWord labelId = READ_WORD();
      ctx->results[labelId] = pc - 2;  // include OpLabel
    } else {
      SKIP_WORD_N(insOp.wordCount - 1);
    }

    insOp = READ_OPCODE();
  }

  // check end of stream
  if ((pc - ctx->module->code) * sizeof(SpvmWord) >= ctx->module->codeSize) {
//    LOGD("init module done");
    ctx->stackBase = sp;
    return Result_InitEnd;
  }
  GO_NEXT
}

DEF_OP_FUNC(SpvOpFunctionParameter) {
  SKIP_WORD(); // result type
  SpvmWord resultId = READ_WORD();
  auto &frame = ctx->currFrame;
  void *param = frame->params[frame->paramsIdx++];
  ctx->results[resultId] = param;
  GO_NEXT
}

DEF_OP_FUNC(SpvOpFunctionEnd) {
  // reset heap ptr
  sp = (SpvmByte *) ctx->currFrame;

  ctx->currFrame = ctx->currFrame->prevFrame;
  if (!ctx->currFrame) {
    // end of entry point function
    return Result_ExecEnd;
  } else {
    pc = ctx->currFrame->pc;
  }
  GO_NEXT
}

DEF_OP_FUNC(SpvOpFunctionCall) {
  SKIP_WORD(); // result type
  SpvmWord resultId = READ_WORD();
  SpvmFunction *func = (SpvmFunction *) ctx->results[READ_WORD()];

  // result
  SpvmTypeFunction *funcType = (SpvmTypeFunction *) func->functionType;
  SpvmValue *retValue = CREATE_VALUE(funcType->returnType);
  ctx->results[resultId] = retValue;

  // create frame
  SpvmFrame *prevFrame = ctx->currFrame;
  ctx->currFrame = (SpvmFrame *) HEAP_MALLOC(sizeof(SpvmFrame));
  ctx->currFrame->prevFrame = prevFrame;
  ctx->currFrame->resultValue = retValue;
  ctx->currFrame->parentBlockId = 0;
  ctx->currFrame->currBlockId = 0;

  // params
  SpvmWord idxCnt = opcode.wordCount - 4;
  void **params = (void **) HEAP_MALLOC(idxCnt * sizeof(void *));
  for (SpvmWord i = 0; i < idxCnt; i++) {
    params[i] = ctx->results[READ_WORD()];
  }
  ctx->currFrame->paramsIdx = 0;
  ctx->currFrame->params = params;

  prevFrame->pc = pc;
  pc = ctx->currFrame->pc = func->code;
  GO_NEXT
}

#define READ_ONE_OPERAND                                                                      \
  SpvmTypeBase *resultType = SPVM_TYPE(READ_WORD());                                          \
  SpvmWord resultId = READ_WORD();                                                            \
  SpvmValue *operand = READ_OPERAND();                                                        \
  SpvmValue *ret = CREATE_VALUE(resultType);                                                  \

#define READ_TWO_OPERAND                                                                      \
  SpvmTypeBase *resultType = SPVM_TYPE(READ_WORD());                                          \
  SpvmWord resultId = READ_WORD();                                                            \
  SpvmValue *operand1 = READ_OPERAND();                                                       \
  SpvmValue *operand2 = READ_OPERAND();                                                       \
  SpvmValue *ret = CREATE_VALUE(resultType);                                                  \


DEF_OP_FUNC(SpvOpSampledImage) {
  READ_TWO_OPERAND
  ret->value.sampledImage->image = operand1->value.image;
  ret->value.sampledImage->sampler = operand2->value.sampler;
  ctx->results[resultId] = ret;
  GO_NEXT
}

DEF_OP_FUNC(SpvOpImageSampleImplicitLod) {
  READ_ONE_OPERAND
  SpvmWord coordinateId = READ_WORD();
  SpvmValue *coordinate = (SpvmValue *) ctx->results[coordinateId];
  SpvmImageOperands operands = readImageOperands(&pc, ctx, opcode.wordCount - 5);
  sampleImage(ctx, ret, operand, coordinate, coordinateId, &operands);
  ctx->results[resultId] = ret;
  GO_NEXT
}

DEF_OP_FUNC(SpvOpImageSampleExplicitLod) {
  READ_ONE_OPERAND
  SpvmWord coordinateId = READ_WORD();
  SpvmValue *coordinate = (SpvmValue *) ctx->results[coordinateId];
  SpvmImageOperands operands = readImageOperands(&pc, ctx, opcode.wordCount - 5);
  sampleImage(ctx, ret, operand, coordinate, coordinateId, &operands);
  ctx->results[resultId] = ret;
  GO_NEXT
}

DEF_OP_FUNC(SpvOpImageSampleDrefImplicitLod) {
  READ_ONE_OPERAND
  SpvmWord coordinateId = READ_WORD();
  SpvmValue *coordinate = (SpvmValue *) ctx->results[coordinateId];
  SpvmValue *depthCompare = READ_OPERAND();
  SpvmImageOperands operands = readImageOperands(&pc, ctx, opcode.wordCount - 6);
  sampleImage(ctx, ret, operand, coordinate, coordinateId, &operands, depthCompare);
  ctx->results[resultId] = ret;
  GO_NEXT
}

DEF_OP_FUNC(SpvOpImageSampleDrefExplicitLod) {
  READ_ONE_OPERAND
  SpvmWord coordinateId = READ_WORD();
  SpvmValue *coordinate = (SpvmValue *) ctx->results[coordinateId];
  SpvmValue *depthCompare = READ_OPERAND();
  SpvmImageOperands operands = readImageOperands(&pc, ctx, opcode.wordCount - 6);
  sampleImage(ctx, ret, operand, coordinate, coordinateId, &operands, depthCompare);
  ctx->results[resultId] = ret;
  GO_NEXT
}

DEF_OP_FUNC(SpvOpImageSampleProjImplicitLod) {
  READ_ONE_OPERAND
  SpvmWord coordinateId = READ_WORD();
  SpvmValue *coordinate = (SpvmValue *) ctx->results[coordinateId];
  SpvmImageOperands operands = readImageOperands(&pc, ctx, opcode.wordCount - 5);
  sampleImage(ctx, ret, operand, coordinate, coordinateId, &operands, nullptr, true);
  ctx->results[resultId] = ret;
  GO_NEXT
}

DEF_OP_FUNC(SpvOpImageSampleProjExplicitLod) {
  READ_ONE_OPERAND
  SpvmWord coordinateId = READ_WORD();
  SpvmValue *coordinate = (SpvmValue *) ctx->results[coordinateId];
  SpvmImageOperands operands = readImageOperands(&pc, ctx, opcode.wordCount - 5);
  sampleImage(ctx, ret, operand, coordinate, coordinateId, &operands, nullptr, true);
  ctx->results[resultId] = ret;
  GO_NEXT
}

DEF_OP_FUNC(SpvOpImageSampleProjDrefImplicitLod) {
  READ_ONE_OPERAND
  SpvmWord coordinateId = READ_WORD();
  SpvmValue *coordinate = (SpvmValue *) ctx->results[coordinateId];
  SpvmValue *depthCompare = READ_OPERAND();
  SpvmImageOperands operands = readImageOperands(&pc, ctx, opcode.wordCount - 6);
  sampleImage(ctx, ret, operand, coordinate, coordinateId, &operands, depthCompare, true);
  ctx->results[resultId] = ret;
  GO_NEXT
}

DEF_OP_FUNC(SpvOpImageSampleProjDrefExplicitLod) {
  READ_ONE_OPERAND
  SpvmWord coordinateId = READ_WORD();
  SpvmValue *coordinate = (SpvmValue *) ctx->results[coordinateId];
  SpvmValue *depthCompare = READ_OPERAND();
  SpvmImageOperands operands = readImageOperands(&pc, ctx, opcode.wordCount - 6);
  sampleImage(ctx, ret, operand, coordinate, coordinateId, &operands, depthCompare, true);
  ctx->results[resultId] = ret;
  GO_NEXT
}

DEF_OP_FUNC(SpvOpImageFetch) {
  READ_ONE_OPERAND
  SpvmWord coordinateId = READ_WORD();
  SpvmValue *coordinate = (SpvmValue *) ctx->results[coordinateId];
  SpvmImageOperands operands = readImageOperands(&pc, ctx, opcode.wordCount - 5);
  fetchImage(ctx, ret, operand, coordinate, coordinateId, &operands);
  ctx->results[resultId] = ret;
  GO_NEXT
}

DEF_OP_FUNC(SpvOpImage) {
  SpvmTypeBase *resultType = SPVM_TYPE(READ_WORD());
  SpvmWord resultId = READ_WORD();
  SpvmValue *sampledImage = READ_OPERAND();
  SpvmValue *ret = (SpvmValue *) HEAP_MALLOC(sizeof(SpvmValue));
  ret->type = resultType;
  ret->memberCnt = resultType->memberCnt;
  ret->value.image = sampledImage->value.sampledImage->image;
  ctx->results[resultId] = ret;
  GO_NEXT
}

DEF_OP_FUNC(SpvOpImageQueryFormat) {
  READ_ONE_OPERAND
  queryImageFormat(ctx, ret, operand);
  ctx->results[resultId] = ret;
  GO_NEXT
}

DEF_OP_FUNC(SpvOpImageQueryOrder) {
  READ_ONE_OPERAND
  queryImageOrder(ctx, ret, operand);
  ctx->results[resultId] = ret;
  GO_NEXT
}

DEF_OP_FUNC(SpvOpImageQuerySizeLod) {
  READ_ONE_OPERAND
  SpvmValue *lod = READ_OPERAND();
  queryImageSizeLod(ctx, ret, operand, lod);
  ctx->results[resultId] = ret;
  GO_NEXT
}

DEF_OP_FUNC(SpvOpImageQuerySize) {
  READ_ONE_OPERAND
  queryImageSize(ctx, ret, operand);
  ctx->results[resultId] = ret;
  GO_NEXT
}

DEF_OP_FUNC(SpvOpImageQueryLod) {
  READ_ONE_OPERAND
  SpvmWord coordinateId = READ_WORD();
  SpvmValue *coordinate = (SpvmValue *) ctx->results[coordinateId];
  queryImageLod(ctx, ret, operand, coordinate, coordinateId);
  ctx->results[resultId] = ret;
  GO_NEXT
}

DEF_OP_FUNC(SpvOpImageQueryLevels) {
  READ_ONE_OPERAND
  queryImageLevels(ctx, ret, operand);
  ctx->results[resultId] = ret;
  GO_NEXT
}

DEF_OP_FUNC(SpvOpImageQuerySamples) {
  READ_ONE_OPERAND
  queryImageSamples(ctx, ret, operand);
  ctx->results[resultId] = ret;
  GO_NEXT
}

#define SPVM_CONVERSION_IMPL(DValue, DType, SValue)                                           \
  READ_ONE_OPERAND                                                                            \
  if (ret->memberCnt > 0) {                                                                   \
    for (SpvmWord i = 0; i < ret->memberCnt; i++) {                                           \
      ret->value.members[i]->value.DValue = (DType) operand->value.members[i]->value.SValue;  \
    }                                                                                         \
  } else {                                                                                    \
    ret->value.DValue = (DType) operand->value.SValue;                                        \
  }                                                                                           \
  ctx->results[resultId] = ret;                                                               \


DEF_OP_FUNC(SpvOpConvertFToU) {
  SPVM_CONVERSION_IMPL(u32, SpvmU32, f32)
  GO_NEXT
}

DEF_OP_FUNC(SpvOpConvertFToS) {
  SPVM_CONVERSION_IMPL(i32, SpvmI32, f32)
  GO_NEXT
}

DEF_OP_FUNC(SpvOpConvertSToF) {
  SPVM_CONVERSION_IMPL(f32, SpvmI32, i32)
  GO_NEXT
}

DEF_OP_FUNC(SpvOpConvertUToF) {
  SPVM_CONVERSION_IMPL(f32, SpvmU32, u32)
  GO_NEXT
}

DEF_OP_FUNC(SpvOpBitcast) {
  SPVM_CONVERSION_IMPL(u32, SpvmU32, u32)
  GO_NEXT
}

DEF_OP_FUNC(SpvOpVectorShuffle) {
  SpvmTypeBase *resultType = SPVM_TYPE(READ_WORD());
  SpvmWord resultId = READ_WORD();
  SpvmValue *vec1 = READ_OPERAND();
  SpvmValue *vec2 = READ_OPERAND();
  SpvmValue *retValue = (SpvmValue *) HEAP_MALLOC(sizeof(SpvmValue));
  retValue->type = resultType;
  retValue->memberCnt = opcode.wordCount - 5;
  retValue->value.members = (SpvmValue **) HEAP_MALLOC(retValue->memberCnt * sizeof(SpvmValue *));
  for (SpvmWord i = 0; i < retValue->memberCnt; i++) {
    SpvmWord idx = READ_WORD();
    if (idx < vec1->memberCnt) {
      retValue->value.members[i] = vec1->value.members[idx];
    } else if (idx < vec1->memberCnt + vec2->memberCnt) {
      retValue->value.members[i] = vec2->value.members[idx - vec1->memberCnt];
    }
  }
  ctx->results[resultId] = retValue;
  GO_NEXT
}

DEF_OP_FUNC(SpvOpCompositeConstruct) {
  SpvmTypeBase *resultType = SPVM_TYPE(READ_WORD());
  SpvmWord resultId = READ_WORD();

  SpvmValue *retValue = (SpvmValue *) HEAP_MALLOC(sizeof(SpvmValue));
  retValue->type = resultType;
  retValue->memberCnt = opcode.wordCount - 3;
  retValue->value.members = (SpvmValue **) HEAP_MALLOC(retValue->memberCnt * sizeof(SpvmValue *));
  for (SpvmWord i = 0; i < retValue->memberCnt; i++) {
    retValue->value.members[i] = READ_OPERAND();
  }
  ctx->results[resultId] = retValue;
  GO_NEXT
}

DEF_OP_FUNC(SpvOpCompositeExtract) {
  SKIP_WORD();
  SpvmWord resultId = READ_WORD();
  SpvmWord compositeId = READ_WORD();

  SpvmValue *composite = (SpvmValue *) ctx->results[compositeId];
  SpvmWord idxCnt = opcode.wordCount - 4;
  while (idxCnt > 0) {
    idxCnt--;
    SpvmWord memberIdx = READ_WORD();
    composite = composite->value.members[memberIdx];
  }
  ctx->results[resultId] = composite;
  GO_NEXT
}

DEF_OP_FUNC(SpvOpCopyObject) {
  SKIP_WORD();
  SpvmWord resultId = READ_WORD();
  SpvmValue *operand = READ_OPERAND();
  SpvmValue *ret = CREATE_VALUE(operand->type);
  copyValue(ret, operand);
  ctx->results[resultId] = ret;
  GO_NEXT
}

DEF_OP_FUNC(SpvOpTranspose) {
  SpvmTypeBase *resultType = SPVM_TYPE(READ_WORD());
  SpvmWord resultId = READ_WORD();
  SpvmValue *operand = READ_OPERAND();
  SpvmValue *ret = CREATE_VALUE(resultType);
  SpvmWord colCnt = ret->memberCnt;
  SpvmWord rowCnt = ret->value.members[0]->memberCnt;
  for (SpvmWord i = 0; i < colCnt; i++) {
    for (SpvmWord j = 0; j < rowCnt; j++) {
      ret->MATRIX_value(i, j) = operand->MATRIX_value(j, i);
    }
  }
  ctx->results[resultId] = ret;
  GO_NEXT
}

#define SPVM_MATH_ONE_IMPL(RetType, OpType, OP)                   \
  READ_ONE_OPERAND                                                \
  if (operand->memberCnt > 0) {                                   \
    for (SpvmWord i = 0; i < operand->memberCnt; i++) {           \
      ret->VECTOR_##RetType(i) = OP(operand->VECTOR_##OpType(i)); \
    }                                                             \
  } else {                                                        \
    ret->value.RetType = OP(operand->value.OpType);               \
  }                                                               \
  ctx->results[resultId] = ret;                                   \

#define SPVM_MATH_TWO_IMPL(RetType, OpType, OP)                                                   \
  READ_TWO_OPERAND                                                                                \
  if (operand1->memberCnt > 0) {                                                                  \
    for (SpvmWord i = 0; i < operand1->memberCnt; i++) {                                          \
      ret->VECTOR_##RetType(i) = (operand1->VECTOR_##OpType(i) OP operand2->VECTOR_##OpType(i));  \
    }                                                                                             \
  } else {                                                                                        \
    ret->value.RetType = (operand1->value.OpType OP operand2->value.OpType);                      \
  }                                                                                               \
  ctx->results[resultId] = ret;                                                                   \

DEF_OP_FUNC(SpvOpSNegate) {
  SPVM_MATH_ONE_IMPL(i32, i32, -)
  GO_NEXT
}

DEF_OP_FUNC(SpvOpFNegate) {
  SPVM_MATH_ONE_IMPL(f32, f32, -)
  GO_NEXT
}

DEF_OP_FUNC(SpvOpIAdd) {
  SPVM_MATH_TWO_IMPL(i32, i32, +)
  GO_NEXT
}

DEF_OP_FUNC(SpvOpFAdd) {
  SPVM_MATH_TWO_IMPL(f32, f32, +)
  GO_NEXT
}

DEF_OP_FUNC(SpvOpISub) {
  SPVM_MATH_TWO_IMPL(i32, i32, -)
  GO_NEXT
}

DEF_OP_FUNC(SpvOpFSub) {
  SPVM_MATH_TWO_IMPL(f32, f32, -)
  GO_NEXT
}

DEF_OP_FUNC(SpvOpIMul) {
  SPVM_MATH_TWO_IMPL(i32, i32, *)
  GO_NEXT
}

DEF_OP_FUNC(SpvOpFMul) {
  SPVM_MATH_TWO_IMPL(f32, f32, *)
  GO_NEXT
}

DEF_OP_FUNC(SpvOpUDiv) {
  SPVM_MATH_TWO_IMPL(u32, u32, /)
  GO_NEXT
}

DEF_OP_FUNC(SpvOpSDiv) {
  SPVM_MATH_TWO_IMPL(i32, i32, /)
  GO_NEXT
}

DEF_OP_FUNC(SpvOpFDiv) {
  SPVM_MATH_TWO_IMPL(f32, f32, /)
  GO_NEXT
}

DEF_OP_FUNC(SpvOpUMod) {
  SPVM_MATH_TWO_IMPL(u32, u32, %)
  GO_NEXT
}

DEF_OP_FUNC(SpvOpSMod) {
  READ_TWO_OPERAND
  if (ret->memberCnt > 0) {
    for (SpvmWord i = 0; i < ret->memberCnt; i++) {
      SpvmI32 rem = operand1->VECTOR_i32(i) % operand2->VECTOR_i32(i);
      if (((rem < 0) != (operand2->VECTOR_i32(i) < 0))) {
        rem = -rem;
      }
      ret->VECTOR_i32(i) = rem;
    }
  } else {
    SpvmI32 rem = operand1->value.i32 % operand2->value.i32;
    if (((rem < 0) != (operand2->value.i32 < 0))) {
      rem = -rem;
    }
    ret->value.i32 = rem;
  }
  ctx->results[resultId] = ret;
  GO_NEXT
}

DEF_OP_FUNC(SpvOpFMod) {
  READ_TWO_OPERAND
  if (ret->memberCnt > 0) {
    for (SpvmWord i = 0; i < ret->memberCnt; i++) {
      SpvmF32 rem = fmod(operand1->VECTOR_f32(i), operand2->VECTOR_f32(i));
      if (((rem < 0) != (operand2->VECTOR_f32(i) < 0))) {
        rem = -rem;
      }
      ret->VECTOR_f32(i) = rem;
    }
  } else {
    SpvmF32 rem = fmod(operand1->value.f32, operand2->value.f32);
    if (((rem < 0) != (operand2->value.f32 < 0))) {
      rem = -rem;
    }
    ret->value.f32 = rem;
  }
  ctx->results[resultId] = ret;
  GO_NEXT
}

DEF_OP_FUNC(SpvOpVectorTimesScalar) {
  READ_TWO_OPERAND
  for (SpvmWord i = 0; i < ret->memberCnt; i++) {
    ret->VECTOR_f32(i) = operand1->VECTOR_f32(i) * operand2->value.f32;
  }
  ctx->results[resultId] = ret;
  GO_NEXT
}

DEF_OP_FUNC(SpvOpMatrixTimesScalar) {
  READ_TWO_OPERAND
  SpvmWord colCnt = ret->memberCnt;
  SpvmWord rowCnt = ret->value.members[0]->memberCnt;
  for (SpvmWord i = 0; i < colCnt; i++) {
    for (SpvmWord j = 0; j < rowCnt; j++) {
      ret->MATRIX_f32(i, j) = operand1->MATRIX_f32(i, j) * operand2->value.f32;
    }
  }
  ctx->results[resultId] = ret;
  GO_NEXT
}

DEF_OP_FUNC(SpvOpVectorTimesMatrix) {
  READ_TWO_OPERAND
  SpvmWord colCnt = operand2->memberCnt;
  SpvmWord rowCnt = operand2->value.members[0]->memberCnt;
  for (SpvmWord i = 0; i < colCnt; i++) {
    ret->VECTOR_f32(i) = 0;
    for (SpvmWord j = 0; j < rowCnt; j++) {
      ret->VECTOR_f32(i) += operand1->VECTOR_f32(j) * operand2->MATRIX_f32(i, j);
    }
  }
  ctx->results[resultId] = ret;
  GO_NEXT
}

DEF_OP_FUNC(SpvOpMatrixTimesVector) {
  READ_TWO_OPERAND
  SpvmWord colCnt = operand1->memberCnt;
  SpvmWord rowCnt = operand1->value.members[0]->memberCnt;
  for (SpvmWord j = 0; j < rowCnt; j++) {
    ret->VECTOR_f32(j) = 0;
    for (SpvmWord i = 0; i < colCnt; i++) {
      ret->VECTOR_f32(j) += operand1->MATRIX_f32(i, j) * operand2->VECTOR_f32(i);
    }
  }
  ctx->results[resultId] = ret;
  GO_NEXT
}

DEF_OP_FUNC(SpvOpMatrixTimesMatrix) {
  READ_TWO_OPERAND
  SpvmWord colCnt = ret->memberCnt;
  SpvmWord rowCnt = ret->value.members[0]->memberCnt;
  for (SpvmWord i = 0; i < colCnt; i++) {
    for (SpvmWord j = 0; j < rowCnt; j++) {
      ret->MATRIX_f32(i, j) = 0;
      for (int k = 0; k < colCnt; k++) {
        ret->MATRIX_f32(i, j) += operand1->MATRIX_f32(k, j) * operand2->MATRIX_f32(i, k);
      }
    }
  }
  ctx->results[resultId] = ret;
  GO_NEXT
}

DEF_OP_FUNC(SpvOpOuterProduct) {
  READ_TWO_OPERAND
  SpvmWord colCnt = ret->memberCnt;
  SpvmWord rowCnt = ret->value.members[0]->memberCnt;
  for (SpvmWord i = 0; i < colCnt; i++) {
    for (SpvmWord j = 0; j < rowCnt; j++) {
      ret->MATRIX_f32(i, j) = operand1->VECTOR_f32(j) * operand2->VECTOR_f32(i);
    }
  }
  ctx->results[resultId] = ret;
  GO_NEXT
}

DEF_OP_FUNC(SpvOpDot) {
  READ_TWO_OPERAND
  ret->value.f32 = 0;
  for (SpvmWord i = 0; i < operand1->memberCnt; i++) {
    ret->value.f32 += operand1->VECTOR_f32(i) * operand2->VECTOR_f32(i);
  }
  ctx->results[resultId] = ret;
  GO_NEXT
}

DEF_OP_FUNC(SpvOpIAddCarry) {
  READ_TWO_OPERAND
  if (operand1->memberCnt > 0) {
    SpvmU64 bit32Max = (1llu << 32) - 1;
    for (SpvmWord i = 0; i < ret->memberCnt; i++) {
      SpvmU64 u64 = (SpvmU64) operand1->VECTOR_u32(i) + (SpvmU64) operand2->VECTOR_u32(i);
      if (u64 > bit32Max) {
        u64 %= bit32Max;
        ret->VECTOR_value(1).members[i]->value.u32 = 1;
      }
      ret->VECTOR_value(0).members[i]->value.u32 = u64;
    }
  } else {
    SpvmU64 bit32Max = (1llu << 32) - 1;
    SpvmU64 u64 = (SpvmU64) operand1->value.u32 + (SpvmU64) operand2->value.u32;
    if (u64 > bit32Max) {
      u64 %= bit32Max;
      ret->value.members[1]->value.u32 = 1;
    }
    ret->value.members[0]->value.u32 = u64;
  }
  ctx->results[resultId] = ret;
  GO_NEXT
}

DEF_OP_FUNC(SpvOpISubBorrow) {
  READ_TWO_OPERAND
  if (operand1->memberCnt > 0) {
    SpvmU64 bit32Max = (1llu << 32) - 1;
    for (SpvmWord i = 0; i < ret->memberCnt; i++) {
      SpvmU64 u641 = operand1->VECTOR_u32(i);
      SpvmU64 u642 = operand2->VECTOR_u32(i);
      if (u641 < u642) {
        u641 += bit32Max;
        ret->VECTOR_value(1).members[i]->value.u32 = 1;
      }
      ret->VECTOR_value(0).members[i]->value.u32 = u641 - u642;
    }
  } else {
    SpvmU64 bit32Max = (1llu << 32) - 1;
    SpvmU64 u641 = operand1->value.u32;
    SpvmU64 u642 = operand2->value.u32;
    if (u641 < u642) {
      u641 += bit32Max;
      ret->value.members[1]->value.u32 = 1;
    }
    ret->value.members[0]->value.u32 = u641 - u642;
  }
  ctx->results[resultId] = ret;
  GO_NEXT
}

DEF_OP_FUNC(SpvOpUMulExtended) {  // only support 32bit operands
  READ_TWO_OPERAND
  if (operand1->memberCnt > 0) {
    SpvmU64 bit32Mask = (1llu << 32) - 1;
    for (SpvmWord i = 0; i < ret->memberCnt; i++) {
      SpvmU64 u64 = operand1->VECTOR_u32(i);
      u64 *= operand2->VECTOR_u32(i);
      ret->VECTOR_value(0).members[i]->value.u32 = (SpvmU32) (u64 & bit32Mask);
      ret->VECTOR_value(1).members[i]->value.u32 = (SpvmU32) ((u64 & (~bit32Mask)) >> 32);
    }
  } else {
    SpvmU64 bit32Mask = (1llu << 32) - 1;
    SpvmU64 u64 = operand1->value.u32;
    u64 *= operand2->value.u32;
    ret->value.members[0]->value.u32 = (SpvmU32) (u64 & bit32Mask);
    ret->value.members[1]->value.u32 = (SpvmU32) ((u64 & (~bit32Mask)) >> 32);
  }
  ctx->results[resultId] = ret;
  GO_NEXT
}

DEF_OP_FUNC(SpvOpSMulExtended) {  // only support 32bit operands
  READ_TWO_OPERAND
  if (operand1->memberCnt > 0) {
    SpvmU64 bit32Mask = (1llu << 32) - 1;
    for (SpvmWord i = 0; i < ret->memberCnt; i++) {
      SpvmI64 i64 = operand1->VECTOR_i32(i);
      i64 *= operand2->VECTOR_i32(i);
      ret->VECTOR_value(0).members[i]->value.i32 = (SpvmI32) (i64 & bit32Mask);
      ret->VECTOR_value(1).members[i]->value.i32 = (SpvmI32) ((i64 & (~bit32Mask)) >> 32);
    }
  } else {
    SpvmU64 bit32Mask = (1llu << 32) - 1;
    SpvmI64 i64 = operand1->value.i32;
    i64 *= operand2->value.i32;
    ret->value.members[0]->value.i32 = (SpvmI32) (i64 & bit32Mask);
    ret->value.members[1]->value.i32 = (SpvmI32) ((i64 & (~bit32Mask)) >> 32);
  }
  ctx->results[resultId] = ret;
  GO_NEXT
}

#define SPVM_SHIFT_IMPL(Type, OP)                                                       \
  READ_TWO_OPERAND                                                                      \
  if (operand1->memberCnt > 0) {                                                        \
    for (SpvmWord i = 0; i < operand1->memberCnt; i++) {                                \
      ret->VECTOR_##Type(i) = operand1->VECTOR_##Type(i) OP operand2->VECTOR_u32(i);    \
    }                                                                                   \
  } else {                                                                              \
    ret->value.Type = operand1->value.Type OP operand2->value.u32;                      \
  }                                                                                     \
  ctx->results[resultId] = ret;                                                         \

DEF_OP_FUNC(SpvOpShiftRightLogical) {
  SPVM_SHIFT_IMPL(u32, >>)
  GO_NEXT
}

DEF_OP_FUNC(SpvOpShiftRightArithmetic) {
  SPVM_SHIFT_IMPL(i32, >>)
  GO_NEXT
}

DEF_OP_FUNC(SpvOpShiftLeftLogical) {
  SPVM_SHIFT_IMPL(u32, <<)
  GO_NEXT
}

DEF_OP_FUNC(SpvOpBitwiseOr) {
  SPVM_MATH_TWO_IMPL(u32, u32, |)
  GO_NEXT
}

DEF_OP_FUNC(SpvOpBitwiseXor) {
  SPVM_MATH_TWO_IMPL(u32, u32, ^)
  GO_NEXT
}

DEF_OP_FUNC(SpvOpBitwiseAnd) {
  SPVM_MATH_TWO_IMPL(u32, u32, &)
  GO_NEXT
}

DEF_OP_FUNC(SpvOpNot) {
  SPVM_MATH_ONE_IMPL(u32, u32, ~)
  GO_NEXT
}

DEF_OP_FUNC(SpvOpBitFieldInsert) {
  READ_ONE_OPERAND
  SpvmValue *insert = READ_OPERAND();
  SpvmValue *offset = READ_OPERAND();
  SpvmValue *count = READ_OPERAND();
  if (ret->memberCnt > 0) {
    for (SpvmWord i = 0; i < ret->memberCnt; i++) {
      ret->VECTOR_u32(i) = bitInsert(operand->VECTOR_u32(i), insert->VECTOR_u32(i),
                                     offset->value.i32, count->value.i32);
    }
  } else {
    ret->value.u32 = bitInsert(operand->value.u32, insert->value.u32,
                               offset->value.i32, count->value.i32);
  }
  ctx->results[resultId] = ret;
  GO_NEXT
}

DEF_OP_FUNC(SpvOpBitFieldSExtract) {
  READ_ONE_OPERAND
  SpvmValue *offset = READ_OPERAND();
  SpvmValue *count = READ_OPERAND();
  if (ret->memberCnt > 0) {
    for (SpvmWord i = 0; i < ret->memberCnt; i++) {
      ret->VECTOR_i32(i) = bitExtractSigned(operand->VECTOR_i32(i), offset->value.i32, count->value.i32);
    }
  } else {
    ret->value.i32 = bitExtractSigned(operand->value.i32, offset->value.i32, count->value.i32);
  }
  ctx->results[resultId] = ret;
  GO_NEXT
}

DEF_OP_FUNC(SpvOpBitFieldUExtract) {
  READ_ONE_OPERAND
  SpvmValue *offset = READ_OPERAND();
  SpvmValue *count = READ_OPERAND();
  if (ret->memberCnt > 0) {
    for (SpvmWord i = 0; i < ret->memberCnt; i++) {
      ret->VECTOR_u32(i) = bitExtractUnsigned(operand->VECTOR_u32(i), offset->value.i32, count->value.i32);
    }
  } else {
    ret->value.u32 = bitExtractUnsigned(operand->value.u32, offset->value.i32, count->value.i32);
  }
  ctx->results[resultId] = ret;
  GO_NEXT
}

DEF_OP_FUNC(SpvOpBitReverse) {
  SPVM_MATH_ONE_IMPL(u32, u32, bitReverse)
  GO_NEXT
}

DEF_OP_FUNC(SpvOpBitCount) {
  SPVM_MATH_ONE_IMPL(u32, u32, bitCount)
  GO_NEXT
}

DEF_OP_FUNC(SpvOpAny) {
  READ_ONE_OPERAND
  ret->value.boolean = false;
  for (SpvmWord i = 0; i < operand->memberCnt; i++) {
    if (operand->VECTOR_boolean(i)) {
      ret->value.boolean = true;
      break;
    }
  }
  ctx->results[resultId] = ret;
  GO_NEXT
}

DEF_OP_FUNC(SpvOpAll) {
  READ_ONE_OPERAND
  ret->value.boolean = true;
  for (SpvmWord i = 0; i < operand->memberCnt; i++) {
    if (!operand->VECTOR_boolean(i)) {
      ret->value.boolean = false;
      break;
    }
  }
  ctx->results[resultId] = ret;
  GO_NEXT
}

DEF_OP_FUNC(SpvOpIsNan) {
  SPVM_MATH_ONE_IMPL(boolean, f32, isnan)
  GO_NEXT
}

DEF_OP_FUNC(SpvOpIsInf) {
  SPVM_MATH_ONE_IMPL(boolean, f32, isinf)
  GO_NEXT
}

DEF_OP_FUNC(SpvOpLogicalEqual) {
  SPVM_MATH_TWO_IMPL(boolean, boolean, ==)
  GO_NEXT
}

DEF_OP_FUNC(SpvOpLogicalNotEqual) {
  SPVM_MATH_TWO_IMPL(boolean, boolean, !=)
  GO_NEXT
}

DEF_OP_FUNC(SpvOpLogicalOr) {
  SPVM_MATH_TWO_IMPL(boolean, boolean, ||)
  GO_NEXT
}

DEF_OP_FUNC(SpvOpLogicalAnd) {
  SPVM_MATH_TWO_IMPL(boolean, boolean, &&)
  GO_NEXT
}

DEF_OP_FUNC(SpvOpLogicalNot) {
  SPVM_MATH_ONE_IMPL(boolean, boolean, !)
  GO_NEXT
}

DEF_OP_FUNC(SpvOpSelect) {
  READ_ONE_OPERAND
  SpvmValue *obj1 = READ_OPERAND();
  SpvmValue *obj2 = READ_OPERAND();
  if (ret->memberCnt > 0) {
    for (SpvmWord i = 0; i < ret->memberCnt; i++) {
      ret->VECTOR_value(i) = operand->VECTOR_boolean(i) ? obj1->VECTOR_value(i) : obj2->VECTOR_value(i);
    }
  } else {
    ret->value = operand->value.boolean ? obj1->value : obj2->value;
  }
  ctx->results[resultId] = ret;
  GO_NEXT
}

DEF_OP_FUNC(SpvOpIEqual) {
  SPVM_MATH_TWO_IMPL(boolean, i32, ==)
  GO_NEXT
}

DEF_OP_FUNC(SpvOpINotEqual) {
  SPVM_MATH_TWO_IMPL(boolean, i32, !=)
  GO_NEXT
}

DEF_OP_FUNC(SpvOpUGreaterThan) {
  SPVM_MATH_TWO_IMPL(boolean, u32, >)
  GO_NEXT
}

DEF_OP_FUNC(SpvOpSGreaterThan) {
  SPVM_MATH_TWO_IMPL(boolean, i32, >)
  GO_NEXT
}

DEF_OP_FUNC(SpvOpUGreaterThanEqual) {
  SPVM_MATH_TWO_IMPL(boolean, u32, >=)
  GO_NEXT
}

DEF_OP_FUNC(SpvOpSGreaterThanEqual) {
  SPVM_MATH_TWO_IMPL(boolean, i32, >=)
  GO_NEXT
}

DEF_OP_FUNC(SpvOpULessThan) {
  SPVM_MATH_TWO_IMPL(boolean, u32, <)
  GO_NEXT
}

DEF_OP_FUNC(SpvOpSLessThan) {
  SPVM_MATH_TWO_IMPL(boolean, i32, <)
  GO_NEXT
}

DEF_OP_FUNC(SpvOpULessThanEqual) {
  SPVM_MATH_TWO_IMPL(boolean, u32, <=)
  GO_NEXT
}

DEF_OP_FUNC(SpvOpSLessThanEqual) {
  SPVM_MATH_TWO_IMPL(boolean, i32, <=)
  GO_NEXT
}

#define SPVM_MATH_FORD_IMPL(OP)                                                       \
  READ_TWO_OPERAND                                                                    \
  if (operand1->memberCnt > 0) {                                                      \
    for (SpvmWord i = 0; i < operand1->memberCnt; i++) {                              \
      SpvmF32 f1 = operand1->VECTOR_f32(i);                                           \
      SpvmF32 f2 = operand2->VECTOR_f32(i);                                           \
      ret->VECTOR_boolean(i) = (!isnan(f1)) && (!isnan(f2)) && (OP);                  \
    }                                                                                 \
  } else {                                                                            \
    SpvmF32 f1 = operand1->value.f32;                                                 \
    SpvmF32 f2 = operand2->value.f32;                                                 \
    ret->value.boolean = (!isnan(f1)) && (!isnan(f2)) && (OP);                        \
  }                                                                                   \
  ctx->results[resultId] = ret;                                                       \


#define SPVM_MATH_FUNORD_IMPL(OP)                                                     \
  READ_TWO_OPERAND                                                                    \
  if (operand1->memberCnt > 0) {                                                      \
    for (SpvmWord i = 0; i < operand1->memberCnt; i++) {                              \
      SpvmF32 f1 = operand1->VECTOR_f32(i);                                           \
      SpvmF32 f2 = operand2->VECTOR_f32(i);                                           \
      ret->VECTOR_boolean(i) = isnan(f1) || isnan(f2) || (OP);                        \
    }                                                                                 \
  } else {                                                                            \
    SpvmF32 f1 = operand1->value.f32;                                                 \
    SpvmF32 f2 = operand2->value.f32;                                                 \
    ret->value.boolean = isnan(f1) || isnan(f2) || (OP);                              \
  }                                                                                   \
  ctx->results[resultId] = ret;                                                       \

DEF_OP_FUNC(SpvOpFOrdEqual) {
  SPVM_MATH_FORD_IMPL(fEqual(f1, f2))
  GO_NEXT
}

DEF_OP_FUNC(SpvOpFUnordEqual) {
  SPVM_MATH_FUNORD_IMPL(fEqual(f1, f2))
  GO_NEXT
}

DEF_OP_FUNC(SpvOpFOrdNotEqual) {
  SPVM_MATH_FORD_IMPL(!fEqual(f1, f2))
  GO_NEXT
}

DEF_OP_FUNC(SpvOpFUnordNotEqual) {
  SPVM_MATH_FUNORD_IMPL(!fEqual(f1, f2))
  GO_NEXT
}

DEF_OP_FUNC(SpvOpFOrdLessThan) {
  SPVM_MATH_FORD_IMPL(f1 < f2)
  GO_NEXT
}

DEF_OP_FUNC(SpvOpFUnordLessThan) {
  SPVM_MATH_FUNORD_IMPL(f1 < f2)
  GO_NEXT
}

DEF_OP_FUNC(SpvOpFOrdGreaterThan) {
  SPVM_MATH_FORD_IMPL(f1 > f2)
  GO_NEXT
}

DEF_OP_FUNC(SpvOpFUnordGreaterThan) {
  SPVM_MATH_FUNORD_IMPL(f1 > f2)
  GO_NEXT
}

DEF_OP_FUNC(SpvOpFOrdLessThanEqual) {
  SPVM_MATH_FORD_IMPL(f1 <= f2)
  GO_NEXT
}

DEF_OP_FUNC(SpvOpFUnordLessThanEqual) {
  SPVM_MATH_FUNORD_IMPL(f1 <= f2)
  GO_NEXT
}

DEF_OP_FUNC(SpvOpFOrdGreaterThanEqual) {
  SPVM_MATH_FORD_IMPL(f1 >= f2)
  GO_NEXT
}

DEF_OP_FUNC(SpvOpFUnordGreaterThanEqual) {
  SPVM_MATH_FUNORD_IMPL(f1 >= f2)
  GO_NEXT
}

#define READ_DERIVATIVE_OPERAND                                                               \
  SpvmTypeBase *resultType = SPVM_TYPE(READ_WORD());                                          \
  SpvmWord resultId = READ_WORD();                                                            \
  SpvmWord P = READ_WORD();                                                                   \

#define DERIVATIVE_WIDTH(dx, dy)                                                              \
  if (ret->memberCnt > 0) {                                                                   \
    for (SpvmWord i = 0; i < ret->memberCnt; i++) {                                           \
      ret->VECTOR_f32(i) = fabs((dx)->VECTOR_f32(i)) + fabs((dy)->VECTOR_f32(i));             \
    }                                                                                         \
  } else {                                                                                    \
    ret->value.f32 = fabs((dx)->value.f32) + fabs((dy)->value.f32);                           \
  }                                                                                           \

DEF_OP_FUNC(SpvOpDPdx) {
  READ_DERIVATIVE_OPERAND
  if (ctx->quadCtx) {
    SpvmValue *dx = ctx->quadCtx->getDPdx(ctx->quadIdx, P);
    ctx->results[resultId] = dx;
  } else {
    ctx->results[resultId] = CREATE_VALUE(resultType);
  }
  GO_NEXT
}

DEF_OP_FUNC(SpvOpDPdy) {
  READ_DERIVATIVE_OPERAND
  if (ctx->quadCtx) {
    SpvmValue *dy = ctx->quadCtx->getDPdy(ctx->quadIdx, P);
    ctx->results[resultId] = dy;
  } else {
    ctx->results[resultId] = CREATE_VALUE(resultType);
  }
  GO_NEXT
}

DEF_OP_FUNC(SpvOpFwidth) {
  READ_DERIVATIVE_OPERAND
  SpvmValue *ret = CREATE_VALUE(resultType);
  if (ctx->quadCtx) {
    SpvmValue *dx = ctx->quadCtx->getDPdx(ctx->quadIdx, P);
    SpvmValue *dy = ctx->quadCtx->getDPdy(ctx->quadIdx, P);
    DERIVATIVE_WIDTH(dx, dy)
  }
  ctx->results[resultId] = ret;
  GO_NEXT
}

DEF_OP_FUNC(SpvOpDPdxFine) {
  READ_DERIVATIVE_OPERAND
  if (ctx->quadCtx) {
    SpvmValue *dx = ctx->quadCtx->getDPdxFine(ctx->quadIdx, P);
    ctx->results[resultId] = dx;
  } else {
    ctx->results[resultId] = CREATE_VALUE(resultType);
  }
  GO_NEXT
}

DEF_OP_FUNC(SpvOpDPdyFine) {
  READ_DERIVATIVE_OPERAND
  if (ctx->quadCtx) {
    SpvmValue *dy = ctx->quadCtx->getDPdyFine(ctx->quadIdx, P);
    ctx->results[resultId] = dy;
  } else {
    ctx->results[resultId] = CREATE_VALUE(resultType);
  }
  GO_NEXT
}

DEF_OP_FUNC(SpvOpFwidthFine) {
  READ_DERIVATIVE_OPERAND
  SpvmValue *ret = CREATE_VALUE(resultType);
  if (ctx->quadCtx) {
    SpvmValue *dx = ctx->quadCtx->getDPdxFine(ctx->quadIdx, P);
    SpvmValue *dy = ctx->quadCtx->getDPdyFine(ctx->quadIdx, P);
    DERIVATIVE_WIDTH(dx, dy)
  }
  ctx->results[resultId] = ret;
  GO_NEXT
}

DEF_OP_FUNC(SpvOpDPdxCoarse) {
  READ_DERIVATIVE_OPERAND
  if (ctx->quadCtx) {
    SpvmValue *dx = ctx->quadCtx->getDPdxCoarse(ctx->quadIdx, P);
    ctx->results[resultId] = dx;
  } else {
    ctx->results[resultId] = CREATE_VALUE(resultType);
  }
  GO_NEXT
}

DEF_OP_FUNC(SpvOpDPdyCoarse) {
  READ_DERIVATIVE_OPERAND
  if (ctx->quadCtx) {
    SpvmValue *dy = ctx->quadCtx->getDPdyCoarse(ctx->quadIdx, P);
    ctx->results[resultId] = dy;
  } else {
    ctx->results[resultId] = CREATE_VALUE(resultType);
  }
  GO_NEXT
}

DEF_OP_FUNC(SpvOpFwidthCoarse) {
  READ_DERIVATIVE_OPERAND
  SpvmValue *ret = CREATE_VALUE(resultType);
  if (ctx->quadCtx) {
    SpvmValue *dx = ctx->quadCtx->getDPdxCoarse(ctx->quadIdx, P);
    SpvmValue *dy = ctx->quadCtx->getDPdyCoarse(ctx->quadIdx, P);
    DERIVATIVE_WIDTH(dx, dy)
  }
  ctx->results[resultId] = ret;
  GO_NEXT
}

DEF_OP_FUNC(SpvOpPhi) {
  SpvmTypeBase *resultType = SPVM_TYPE(READ_WORD());
  SpvmWord resultId = READ_WORD();
  SpvmValue *ret = CREATE_VALUE(resultType);
  SpvmWord pairCnt = (opcode.wordCount - 3) / 2;
  for (SpvmWord i = 0; i < pairCnt; i++) {
    SpvmWord valId = READ_WORD();
    SpvmWord labelId = READ_WORD();
    if (ctx->currFrame->parentBlockId == labelId) {
      copyValue(ret, (SpvmValue *) ctx->results[valId]);
    }
  }
  ctx->results[resultId] = ret;
  GO_NEXT
}

DEF_OP_FUNC(SpvOpLoopMerge) {
  SKIP_WORD_N(opcode.wordCount - 1);
  // skip
  GO_NEXT
}

DEF_OP_FUNC(SpvOpSelectionMerge) {
  SKIP_WORD_N(opcode.wordCount - 1);
  // skip
  GO_NEXT
}

DEF_OP_FUNC(SpvOpLabel) {
  SpvmWord labelId = READ_WORD();
  ctx->currFrame->parentBlockId = ctx->currFrame->currBlockId;
  ctx->currFrame->currBlockId = labelId;
  GO_NEXT
}

DEF_OP_FUNC(SpvOpBranch) {
  SpvmWord labelId = READ_WORD();
  pc = (SpvmWord *) ctx->results[labelId];
  GO_NEXT
}

DEF_OP_FUNC(SpvOpBranchConditional) {
  SpvmWord conditionId = READ_WORD();
  SpvmWord trueLabelId = READ_WORD();
  SpvmWord falseLabelId = READ_WORD();
  SKIP_WORD_N(opcode.wordCount - 4);  // ignore branch weights

  SpvmValue *conditionValue = (SpvmValue *) ctx->results[conditionId];
  pc = (SpvmWord *) ctx->results[conditionValue->value.boolean ? trueLabelId : falseLabelId];
  GO_NEXT
}

DEF_OP_FUNC(SpvOpSwitch) {
  SpvmValue *selectorValue = READ_OPERAND();
  SpvmWord retLabelId = READ_WORD();  // default
  SpvmWord caseCnt = (opcode.wordCount - 3) / 2;
  for (SpvmWord i = 0; i < caseCnt; i++) {
    SpvmI32 caseVal = READ_WORD();
    SpvmWord labelId = READ_WORD();
    if (caseVal == selectorValue->value.i32) {
      retLabelId = labelId;
    }
  }

  pc = (SpvmWord *) ctx->results[retLabelId];
  GO_NEXT
}

DEF_OP_FUNC(SpvOpKill) {
  return Result_Killed;
}

DEF_OP_FUNC(SpvOpReturn) {
  GO_NEXT
}

DEF_OP_FUNC(SpvOpReturnValue) {
  SpvmValue *retValue = READ_OPERAND();
  if (retValue) {
    auto &frame = ctx->currFrame;
    COPY_VALUE(frame->resultValue, retValue);
  }
  GO_NEXT
}

DEF_OP_FUNC(SpvOpUnreachable) {
  LOGE("SpvOpUnreachable");
  return Result_Unreachable;
}

}