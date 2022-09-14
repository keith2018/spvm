/*
 * spvm
 * @author 	: keith@robot9.me
 *
 */

#include "quad.h"
#include "logger.h"

namespace SPVM {

RuntimeQuadContext::RuntimeQuadContext() = default;

RuntimeQuadContext::~RuntimeQuadContext() {
  if (heap_) {
    delete[] heap_;
    heap_ = nullptr;
  }
}

bool RuntimeQuadContext::initWithModule(SpvmModule *module, SpvmWord heapSize) {
  if (heapSize_ != 0) {
    LOGE("initWithModule already inited");
    return false;
  }

  module_ = module;
  heapSize_ = heapSize;
  heap_ = new SpvmByte[heapSize_];

  dxdy_ = (DerivativeRet *) heap_;
  stackBase_ = (void **) (heap_ + module->bound * sizeof(DerivativeRet));

  bool ret = true;
  for (auto &rt : rts_) {
    ret = ret && rt.initWithModule(module, heapSize);
  }
  return ret;
}

bool RuntimeQuadContext::execEntryPoint(SpvmWord entryIdx) {
  // reset heap
  memset(heap_, 0, heapSize_ * sizeof(SpvmByte));
  sp_ = (SpvmByte *) stackBase_;

  bool ret = true;
  for (auto &rt : rts_) {
    ret = ret && rt.execEntryPoint(entryIdx);
  }
  return ret;
}

SpvmValue *RuntimeQuadContext::getDPdxFine(SpvmWord quadIdx, SpvmWord P) {
  DerivativeRet *d = &dxdy_[P];
  if (d->dx[quadIdx] == nullptr) {
    evalDPdxFine(P);
  }
  return d->dx[quadIdx];
}

SpvmValue *RuntimeQuadContext::getDPdyFine(SpvmWord quadIdx, SpvmWord P) {
  DerivativeRet *d = &dxdy_[P];
  if (d->dy[quadIdx] == nullptr) {
    evalDPdyFine(P);
  }
  return d->dy[quadIdx];
}

SpvmValue *RuntimeQuadContext::getDPdxCoarse(SpvmWord quadIdx, SpvmWord P) {
  DerivativeRet *d = &dxdy_[P];
  if (d->dx[quadIdx] == nullptr) {
    evalDPdxCoarse(P);
  }
  return d->dx[quadIdx];
}

SpvmValue *RuntimeQuadContext::getDPdyCoarse(SpvmWord quadIdx, SpvmWord P) {
  DerivativeRet *d = &dxdy_[P];
  if (d->dy[quadIdx] == nullptr) {
    evalDPdyCoarse(P);
  }
  return d->dy[quadIdx];
}

SpvmValue *RuntimeQuadContext::getResultValue(SpvmWord quadIdx, SpvmWord P) {
  return (SpvmValue *) rts_[quadIdx].getRuntimeContext().results[P];
}

void RuntimeQuadContext::evalDPdxFine(SpvmWord P) {
  syncQuadForValue(P);

  SpvmValue *v0 = getResultValue(0, P);
  SpvmValue *v1 = getResultValue(1, P);
  SpvmValue *v2 = getResultValue(2, P);
  SpvmValue *v3 = getResultValue(3, P);

  SpvmValue *dx0 = createValue(v0->type, &sp_);
  SpvmValue *dx2 = createValue(v0->type, &sp_);
  valueSubF32(dx0, v1, v0);
  valueSubF32(dx2, v3, v2);

  DerivativeRet *d = &dxdy_[P];
  d->dx[0] = dx0;
  d->dx[1] = dx0;
  d->dx[2] = dx2;
  d->dx[3] = dx2;
}

void RuntimeQuadContext::evalDPdyFine(SpvmWord P) {
  syncQuadForValue(P);

  SpvmValue *v0 = getResultValue(0, P);
  SpvmValue *v1 = getResultValue(1, P);
  SpvmValue *v2 = getResultValue(2, P);
  SpvmValue *v3 = getResultValue(3, P);

  SpvmValue *dy0 = createValue(v0->type, &sp_);
  SpvmValue *dy1 = createValue(v0->type, &sp_);
  valueSubF32(dy0, v2, v0);
  valueSubF32(dy1, v3, v1);

  DerivativeRet *d = &dxdy_[P];
  d->dy[0] = dy0;
  d->dy[1] = dy1;
  d->dy[2] = dy0;
  d->dy[3] = dy1;
}

void RuntimeQuadContext::evalDPdxCoarse(SpvmWord P) {
  syncQuadForValue(P);

  SpvmValue *v0 = getResultValue(0, P);
  SpvmValue *v1 = getResultValue(1, P);

  SpvmValue *dx0 = createValue(v0->type, &sp_);
  valueSubF32(dx0, v1, v0);

  DerivativeRet *d = &dxdy_[P];
  d->dx[0] = dx0;
  d->dx[1] = dx0;
  d->dx[2] = dx0;
  d->dx[3] = dx0;
}

void RuntimeQuadContext::evalDPdyCoarse(SpvmWord P) {
  syncQuadForValue(P);

  SpvmValue *v0 = getResultValue(0, P);
  SpvmValue *v2 = getResultValue(2, P);

  SpvmValue *dy0 = createValue(v0->type, &sp_);
  valueSubF32(dy0, v2, v0);

  DerivativeRet *d = &dxdy_[P];
  d->dy[0] = dy0;
  d->dy[1] = dy0;
  d->dy[2] = dy0;
  d->dy[3] = dy0;
}

void RuntimeQuadContext::syncQuadForValue(SpvmWord P) {
  // TODO
}

}
