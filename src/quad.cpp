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
  for (SpvmWord i = 0; i < 4; i++) {
    ret = ret && rts_[i].initWithModule(module, heapSize, this, i);
  }
  return ret;
}

bool RuntimeQuadContext::execEntryPoint(SpvmWord entryIdx) {
  // reset heap
  memset(heap_, 0, heapSize_ * sizeof(SpvmByte));
  sp_ = (SpvmByte *) stackBase_;

  bool ret = true;
  for (auto &rt : rts_) {
    ret = ret && rt.execPrepare(entryIdx);
  }

  for (auto &rt : rts_) {
    ret = ret && rt.execContinue(SpvmResultIdInvalid);
  }
  return ret;
}

SpvmValue *RuntimeQuadContext::getDPdxFine(SpvmWord quadIdx, SpvmWord P) {
  DerivativeRet *d = &dxdy_[P];
  if (d->dx[quadIdx] == nullptr) {
    evalDPdxFine(quadIdx, P);
  }
  return d->dx[quadIdx];
}

SpvmValue *RuntimeQuadContext::getDPdyFine(SpvmWord quadIdx, SpvmWord P) {
  DerivativeRet *d = &dxdy_[P];
  if (d->dy[quadIdx] == nullptr) {
    evalDPdyFine(quadIdx, P);
  }
  return d->dy[quadIdx];
}

SpvmValue *RuntimeQuadContext::getDPdxCoarse(SpvmWord quadIdx, SpvmWord P) {
  DerivativeRet *d = &dxdy_[P];
  if (d->dx[quadIdx] == nullptr) {
    evalDPdxCoarse(quadIdx, P);
  }
  return d->dx[quadIdx];
}

SpvmValue *RuntimeQuadContext::getDPdyCoarse(SpvmWord quadIdx, SpvmWord P) {
  DerivativeRet *d = &dxdy_[P];
  if (d->dy[quadIdx] == nullptr) {
    evalDPdyCoarse(quadIdx, P);
  }
  return d->dy[quadIdx];
}

SpvmValue *RuntimeQuadContext::getResultValue(SpvmWord quadIdx, SpvmWord P) {
  return (SpvmValue *) rts_[quadIdx].getRuntimeContext().results[P];
}

void RuntimeQuadContext::evalDPdxFine(SpvmWord quadIdx, SpvmWord P) {
  syncQuadForResult(quadIdx, P);
  DerivativeRet *d = &dxdy_[P];

  SpvmValue *v0 = getResultValue(0, P);
  SpvmValue *v1 = getResultValue(1, P);
  SpvmValue *v2 = getResultValue(2, P);
  SpvmValue *v3 = getResultValue(3, P);

  if (v0 && v1) {
    SpvmValue *dx0 = createValue(v0->type, &sp_);
    valueSubF32(dx0, v1, v0);
    d->dx[0] = dx0;
    d->dx[1] = dx0;
  }

  if (v2 && v3) {
    SpvmValue *dx2 = createValue(v2->type, &sp_);
    valueSubF32(dx2, v3, v2);
    d->dx[2] = dx2;
    d->dx[3] = dx2;
  }
}

void RuntimeQuadContext::evalDPdyFine(SpvmWord quadIdx, SpvmWord P) {
  syncQuadForResult(quadIdx, P);
  DerivativeRet *d = &dxdy_[P];

  SpvmValue *v0 = getResultValue(0, P);
  SpvmValue *v1 = getResultValue(1, P);
  SpvmValue *v2 = getResultValue(2, P);
  SpvmValue *v3 = getResultValue(3, P);

  if (v0 && v2) {
    SpvmValue *dy0 = createValue(v0->type, &sp_);
    valueSubF32(dy0, v2, v0);
    d->dy[0] = dy0;
    d->dy[2] = dy0;
  }

  if (v1 && v3) {
    SpvmValue *dy1 = createValue(v1->type, &sp_);
    valueSubF32(dy1, v3, v1);
    d->dy[1] = dy1;
    d->dy[3] = dy1;
  }
}

void RuntimeQuadContext::evalDPdxCoarse(SpvmWord quadIdx, SpvmWord P) {
  syncQuadForResult(quadIdx, P);

  SpvmValue *v0 = getResultValue(0, P);
  SpvmValue *v1 = getResultValue(1, P);

  if (v0 && v1) {
    SpvmValue *dx0 = createValue(v0->type, &sp_);
    valueSubF32(dx0, v1, v0);

    DerivativeRet *d = &dxdy_[P];
    d->dx[0] = dx0;
    d->dx[1] = dx0;
    d->dx[2] = dx0;
    d->dx[3] = dx0;
  }
}

void RuntimeQuadContext::evalDPdyCoarse(SpvmWord quadIdx, SpvmWord P) {
  syncQuadForResult(quadIdx, P);

  SpvmValue *v0 = getResultValue(0, P);
  SpvmValue *v2 = getResultValue(2, P);

  if (v0 && v2) {
    SpvmValue *dy0 = createValue(v0->type, &sp_);
    valueSubF32(dy0, v2, v0);

    DerivativeRet *d = &dxdy_[P];
    d->dy[0] = dy0;
    d->dy[1] = dy0;
    d->dy[2] = dy0;
    d->dy[3] = dy0;
  }
}

bool RuntimeQuadContext::syncQuadForResult(SpvmWord quadIdx, SpvmWord P) {
  bool ret = true;
  for (SpvmWord i = 0; i < 4; i++) {
    if (i == quadIdx) {
      continue;
    }
    ret = ret && rts_[i].execContinue(P);
  }
  return ret;
}

}
