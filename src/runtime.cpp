/*
 * spvm
 * @author 	: keith@robot9.me
 *
 */

#include "runtime.h"
#include "opcodes.inc"

namespace SPVM {

Runtime::Runtime() : heapSize_(0), heap_(nullptr) {}

Runtime::~Runtime() {
  if (heap_) {
    delete[] heap_;
    heap_ = nullptr;
  }
}

bool Runtime::initWithModule(SpvmModule *module, SpvmWord heapSize, SpvmByte *heap) {
  if (heapSize_ != 0) {
    LOGE("initWithModule already inited");
    return false;
  }

  if (heapSize <= 0) {
    LOGE("initWithModule error heapSize: %d", heapSize);
    return false;
  }

  heapSize_ = heapSize;
  SpvmByte *heapBase = heap;
  if (heapBase == nullptr) {
    heap_ = new SpvmByte[heapSize_];
    memset(heap_, 0, heapSize_ * sizeof(SpvmByte));
    heapBase = heap_;
  } else {
    heap_ = nullptr;
  }

  ctx_.module = module;
  ctx_.resultsInit = (void **) heapBase;
  ctx_.results = (void **) (heapBase + module->bound * sizeof(void *));
  ctx_.currFrame = nullptr;

  ctx_.pc = ctx_.module->code;
  ctx_.sp = (SpvmByte *) ctx_.results + module->bound * sizeof(void *);

  execRet_ = Result_NoError;
  bool success = execContinue();
  if (!success) {
    LOGE("initWithModule failed");
    return false;
  }

  // store init results status
  memcpy(ctx_.resultsInit, ctx_.results, module->bound * sizeof(void *));

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

  // load init results status
  memcpy(ctx_.results, ctx_.resultsInit, ctx_.module->bound * sizeof(void *));

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

bool Runtime::execContinue() {
  if (execRet_ == Result_ExecEnd) {
    return true;
  }
  RuntimeContext *ctx = &ctx_;

  SpvmWord *pc = ctx->pc;
  SpvmByte *sp = ctx->sp;
  SpvmOpcode opcode = READ_OPCODE();
  execRet_ = execOpCodes(pc, sp, opcode, ctx);

  return execRet_ != Result_Error;
}

}
