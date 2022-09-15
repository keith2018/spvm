/*
 * spvm
 * @author 	: keith@robot9.me
 *
 */

#include "runtime.h"
#include "opcodes.inc"

namespace SPVM {

static RuntimeResult func_SpvOp_NotSupport(SpvmWord *pc, SpvmByte *sp, SpvmOpcode opcode, RuntimeContext *ctx) {
  LOGD("Instruction not support, skip. opcode: %s, size: %d", spvmOpString(opcode.op), opcode.wordCount);
  SKIP_CURRENT_OPCODE();
  GO_NEXT
}

Runtime::Runtime() : heapSize_(0), heap_(nullptr) {
  // init all slot with 'func_SpvOp_NotSupport'
  for (auto &func : __opFuncs) {
    func = func_SpvOp_NotSupport;
  }

  // Miscellaneous Instructions
  REG_OP_FUNC(SpvOpNop)

  // Debug Instructions
  REG_OP_FUNC(SpvOpSource)
  REG_OP_FUNC(SpvOpSourceExtension)
  REG_OP_FUNC(SpvOpName)
  REG_OP_FUNC(SpvOpMemberName)

  // Annotation Instructions
  REG_OP_FUNC(SpvOpDecorate)
  REG_OP_FUNC(SpvOpMemberDecorate)

  // Extension Instructions
  REG_OP_FUNC(SpvOpExtension)
  REG_OP_FUNC(SpvOpExtInstImport)
  REG_OP_FUNC(SpvOpExtInst)

  // Mode-Setting Instructions
  REG_OP_FUNC(SpvOpMemoryModel)
  REG_OP_FUNC(SpvOpEntryPoint)
  REG_OP_FUNC(SpvOpExecutionMode)
  REG_OP_FUNC(SpvOpCapability)

  // Type-Declaration Instructions
  REG_OP_FUNC(SpvOpTypeVoid)
  REG_OP_FUNC(SpvOpTypeBool)
  REG_OP_FUNC(SpvOpTypeInt)
  REG_OP_FUNC(SpvOpTypeFloat)
  REG_OP_FUNC(SpvOpTypeVector)
  REG_OP_FUNC(SpvOpTypeMatrix)
  REG_OP_FUNC(SpvOpTypeImage)
  REG_OP_FUNC(SpvOpTypeSampler)
  REG_OP_FUNC(SpvOpTypeSampledImage)
  REG_OP_FUNC(SpvOpTypeArray)
  REG_OP_FUNC(SpvOpTypeRuntimeArray)
  REG_OP_FUNC(SpvOpTypeStruct)
  REG_OP_FUNC(SpvOpTypePointer)
  REG_OP_FUNC(SpvOpTypeFunction)

  // Constant-Creation Instructions
  REG_OP_FUNC(SpvOpConstantTrue)
  REG_OP_FUNC(SpvOpConstantFalse)
  REG_OP_FUNC(SpvOpConstant)
  REG_OP_FUNC(SpvOpConstantComposite)
  REG_OP_FUNC(SpvOpConstantSampler)
  REG_OP_FUNC(SpvOpConstantNull)

  // Memory Instructions
  REG_OP_FUNC(SpvOpVariable)
  REG_OP_FUNC(SpvOpLoad)
  REG_OP_FUNC(SpvOpStore)
  REG_OP_FUNC(SpvOpAccessChain)
  REG_OP_FUNC(SpvOpArrayLength)

  // Function Instructions
  REG_OP_FUNC(SpvOpFunction)
  REG_OP_FUNC(SpvOpFunctionParameter)
  REG_OP_FUNC(SpvOpFunctionEnd)
  REG_OP_FUNC(SpvOpFunctionCall)

  // Image Instructions
  REG_OP_FUNC(SpvOpSampledImage)
  REG_OP_FUNC(SpvOpImageSampleImplicitLod)
  REG_OP_FUNC(SpvOpImageSampleExplicitLod)
  REG_OP_FUNC(SpvOpImageSampleDrefImplicitLod)
  REG_OP_FUNC(SpvOpImageSampleDrefExplicitLod)
  REG_OP_FUNC(SpvOpImageSampleProjImplicitLod)
  REG_OP_FUNC(SpvOpImageSampleProjExplicitLod)
  REG_OP_FUNC(SpvOpImageSampleProjDrefImplicitLod)
  REG_OP_FUNC(SpvOpImageSampleProjDrefExplicitLod)
  REG_OP_FUNC(SpvOpImageFetch)
  REG_OP_FUNC(SpvOpImage)
  REG_OP_FUNC(SpvOpImageQueryFormat)
  REG_OP_FUNC(SpvOpImageQueryOrder)
  REG_OP_FUNC(SpvOpImageQuerySizeLod)
  REG_OP_FUNC(SpvOpImageQuerySize)
  REG_OP_FUNC(SpvOpImageQueryLod)
  REG_OP_FUNC(SpvOpImageQueryLevels)
  REG_OP_FUNC(SpvOpImageQuerySamples)

  // Conversion Instructions
  REG_OP_FUNC(SpvOpConvertFToU)
  REG_OP_FUNC(SpvOpConvertFToS)
  REG_OP_FUNC(SpvOpConvertSToF)
  REG_OP_FUNC(SpvOpConvertUToF)
  REG_OP_FUNC(SpvOpBitcast)

  // Composite Instructions
  REG_OP_FUNC(SpvOpVectorShuffle)
  REG_OP_FUNC(SpvOpCompositeConstruct)
  REG_OP_FUNC(SpvOpCompositeExtract)
  REG_OP_FUNC(SpvOpCopyObject)
  REG_OP_FUNC(SpvOpTranspose)

  // Arithmetic Instructions
  REG_OP_FUNC(SpvOpSNegate)
  REG_OP_FUNC(SpvOpFNegate)
  REG_OP_FUNC(SpvOpIAdd)
  REG_OP_FUNC(SpvOpFAdd)
  REG_OP_FUNC(SpvOpISub)
  REG_OP_FUNC(SpvOpFSub)
  REG_OP_FUNC(SpvOpIMul)
  REG_OP_FUNC(SpvOpFMul)
  REG_OP_FUNC(SpvOpUDiv)
  REG_OP_FUNC(SpvOpSDiv)
  REG_OP_FUNC(SpvOpFDiv)
  REG_OP_FUNC(SpvOpUMod)
  REG_OP_FUNC(SpvOpSMod)
  REG_OP_FUNC(SpvOpFMod)
  REG_OP_FUNC(SpvOpVectorTimesScalar)
  REG_OP_FUNC(SpvOpMatrixTimesScalar)
  REG_OP_FUNC(SpvOpVectorTimesMatrix)
  REG_OP_FUNC(SpvOpMatrixTimesVector)
  REG_OP_FUNC(SpvOpMatrixTimesMatrix)
  REG_OP_FUNC(SpvOpOuterProduct)
  REG_OP_FUNC(SpvOpDot)
  REG_OP_FUNC(SpvOpIAddCarry)
  REG_OP_FUNC(SpvOpISubBorrow)
  REG_OP_FUNC(SpvOpUMulExtended)
  REG_OP_FUNC(SpvOpSMulExtended)

  // Bit Instructions
  REG_OP_FUNC(SpvOpShiftRightLogical)
  REG_OP_FUNC(SpvOpShiftRightArithmetic)
  REG_OP_FUNC(SpvOpShiftLeftLogical)
  REG_OP_FUNC(SpvOpBitwiseOr)
  REG_OP_FUNC(SpvOpBitwiseXor)
  REG_OP_FUNC(SpvOpBitwiseAnd)
  REG_OP_FUNC(SpvOpNot)
  REG_OP_FUNC(SpvOpBitFieldInsert)
  REG_OP_FUNC(SpvOpBitFieldSExtract)
  REG_OP_FUNC(SpvOpBitFieldUExtract)
  REG_OP_FUNC(SpvOpBitReverse)
  REG_OP_FUNC(SpvOpBitCount)

  // Relational and Logical Instructions
  REG_OP_FUNC(SpvOpAny)
  REG_OP_FUNC(SpvOpAll)
  REG_OP_FUNC(SpvOpIsNan)
  REG_OP_FUNC(SpvOpIsInf)
  REG_OP_FUNC(SpvOpLogicalEqual)
  REG_OP_FUNC(SpvOpLogicalNotEqual)
  REG_OP_FUNC(SpvOpLogicalOr)
  REG_OP_FUNC(SpvOpLogicalAnd)
  REG_OP_FUNC(SpvOpLogicalNot)
  REG_OP_FUNC(SpvOpSelect)
  REG_OP_FUNC(SpvOpIEqual)
  REG_OP_FUNC(SpvOpINotEqual)
  REG_OP_FUNC(SpvOpUGreaterThan)
  REG_OP_FUNC(SpvOpSGreaterThan)
  REG_OP_FUNC(SpvOpUGreaterThanEqual)
  REG_OP_FUNC(SpvOpSGreaterThanEqual)
  REG_OP_FUNC(SpvOpULessThan)
  REG_OP_FUNC(SpvOpSLessThan)
  REG_OP_FUNC(SpvOpULessThanEqual)
  REG_OP_FUNC(SpvOpSLessThanEqual)
  REG_OP_FUNC(SpvOpFOrdEqual)
  REG_OP_FUNC(SpvOpFUnordEqual)
  REG_OP_FUNC(SpvOpFOrdNotEqual)
  REG_OP_FUNC(SpvOpFUnordNotEqual)
  REG_OP_FUNC(SpvOpFOrdLessThan)
  REG_OP_FUNC(SpvOpFUnordLessThan)
  REG_OP_FUNC(SpvOpFOrdGreaterThan)
  REG_OP_FUNC(SpvOpFUnordGreaterThan)
  REG_OP_FUNC(SpvOpFOrdLessThanEqual)
  REG_OP_FUNC(SpvOpFUnordLessThanEqual)
  REG_OP_FUNC(SpvOpFOrdGreaterThanEqual)
  REG_OP_FUNC(SpvOpFUnordGreaterThanEqual)

  // Derivative Instructions
  REG_OP_FUNC(SpvOpDPdx)
  REG_OP_FUNC(SpvOpDPdy)
  REG_OP_FUNC(SpvOpFwidth)
  REG_OP_FUNC(SpvOpDPdxFine)
  REG_OP_FUNC(SpvOpDPdyFine)
  REG_OP_FUNC(SpvOpFwidthFine)
  REG_OP_FUNC(SpvOpDPdxCoarse)
  REG_OP_FUNC(SpvOpDPdyCoarse)
  REG_OP_FUNC(SpvOpFwidthCoarse)

  // Control-Flow Instructions
  REG_OP_FUNC(SpvOpPhi)
  REG_OP_FUNC(SpvOpLoopMerge)
  REG_OP_FUNC(SpvOpSelectionMerge)
  REG_OP_FUNC(SpvOpLabel)
  REG_OP_FUNC(SpvOpBranch)
  REG_OP_FUNC(SpvOpBranchConditional)
  REG_OP_FUNC(SpvOpSwitch)
  REG_OP_FUNC(SpvOpKill)
  REG_OP_FUNC(SpvOpReturn)
  REG_OP_FUNC(SpvOpReturnValue)
  REG_OP_FUNC(SpvOpUnreachable)

  // Atomic Instructions
  // Primitive Instructions
  // Barrier Instructions
  // Group Instructions
  // Device-Side Enqueue Instructions
  // Pipe Instructions
}

Runtime::~Runtime() {
  if (heap_) {
    delete[] heap_;
    heap_ = nullptr;
  }
}

bool Runtime::initWithModule(SpvmModule *module, SpvmWord heapSize,
                             RuntimeQuadContext *quadCtx, SpvmWord quadIdx) {
  if (heapSize_ != 0) {
    LOGE("initWithModule already inited");
    return false;
  }

  execRet_ = Result_NoError;
  heapSize_ = heapSize;
  heap_ = new SpvmByte[heapSize_];
  memset(heap_, 0, heapSize_ * sizeof(SpvmByte));

  ctx_.quadCtx = quadCtx;
  ctx_.quadIdx = quadIdx;
  ctx_.module = module;
  ctx_.results = (void **) heap_;
  ctx_.currFrame = nullptr;

  ctx_.pc = ctx_.module->code;
  ctx_.sp = heap_ + module->bound * sizeof(void *);

  bool success = execContinue();
  if (!success) {
    LOGE("initWithModule failed");
    return false;
  }
  module->inited = true;
  interface_.init(&ctx_);
  return true;
}

bool Runtime::execEntryPoint(SpvmWord entryIdx) {
  bool success = execPrepare(entryIdx);
  if (!success) {
    LOGE("prepare exec failed");
    return false;
  }

  return execContinue();
}

bool Runtime::execPrepare(SpvmWord entryIdx) {
  if (ctx_.module->entryPoints.empty()) {
    LOGE("execPrepare error: no entry point defined");
    return false;
  }

  auto &entry = ctx_.module->entryPoints[entryIdx];
  auto *func = (SpvmFunction *) ctx_.results[entry.id];
  ctx_.sp = ctx_.stackBase;
  ctx_.pc = func->code;

  SpvmWord *pc = ctx_.pc;
  SpvmByte *sp = ctx_.sp;

  // create frame
  ctx_.currFrame = (SpvmFrame *) HEAP_MALLOC(sizeof(SpvmFrame));
  ctx_.currFrame->prevFrame = nullptr;
  ctx_.currFrame->paramsIdx = 0;
  ctx_.currFrame->params = nullptr;
  ctx_.currFrame->resultValue = nullptr;
  ctx_.currFrame->parentBlockId = 0;
  ctx_.currFrame->currBlockId = 0;
  ctx_.currFrame->pc = pc;

  // write back
  ctx_.pc = pc;
  ctx_.sp = sp;

  execRet_ = Result_NoError;
  return true;
}

bool Runtime::execContinue(SpvmWord untilResult) {
  if (execRet_ == Result_ExecEnd) {
    return true;
  }
  RuntimeContext *ctx = &ctx_;

#ifdef SPVM_OP_DISPATCH_TAIL_CALL
  SpvmWord *pc = ctx->pc;
  SpvmByte *sp = ctx->sp;
  SpvmOpcode opcode = READ_OPCODE();
  execRet_ = __opFuncs[opcode.op](pc, sp, opcode, ctx);

  // write back
  ctx->pc = pc;
  ctx->sp = sp;
#else
  while (execRet_ == Result_NoError) {
    SpvmWord *pc = ctx->pc;
    SpvmByte *sp = ctx->sp;
    SpvmOpcode opcode = READ_OPCODE();
    execRet_ = __opFuncs[opcode.op](pc, sp, opcode, ctx);
  }
#endif

  return execRet_ != Result_Error;
}

}
