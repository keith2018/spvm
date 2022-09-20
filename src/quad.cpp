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
  heap_ = new SpvmByte[heapSize_ * 5];  // [quadCtx][rt0][rt1][rt2][rt3]
  memset(heap_, 0, heapSize_ * 5 * sizeof(SpvmByte));

  dxdy_ = (DerivativeRet *) heap_;
  stackBase_ = (void **) (heap_ + module->bound * sizeof(DerivativeRet));

  bool ret = true;
  for (SpvmWord i = 0; i < 4; i++) {
    ret = ret && rts_[i].initWithModule(module, heapSize, heap_ + ((i + 1) * heapSize_), this, i);
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

void RuntimeQuadContext::evalDPdxFine(SpvmWord quadIdx, SpvmWord P) {
  syncQuadForResult(quadIdx, P);
  DerivativeRet *d = &dxdy_[P];

  SpvmValue *v0 = d->val[0];
  SpvmValue *v1 = d->val[1];
  SpvmValue *v2 = d->val[2];
  SpvmValue *v3 = d->val[3];

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

  SpvmValue *v0 = d->val[0];
  SpvmValue *v1 = d->val[1];
  SpvmValue *v2 = d->val[2];
  SpvmValue *v3 = d->val[3];

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
  DerivativeRet *d = &dxdy_[P];

  SpvmValue *v0 = d->val[0];
  SpvmValue *v1 = d->val[1];

  if (v0 && v1) {
    SpvmValue *dx0 = createValue(v0->type, &sp_);
    valueSubF32(dx0, v1, v0);

    d->dx[0] = dx0;
    d->dx[1] = dx0;
    d->dx[2] = dx0;
    d->dx[3] = dx0;
  }
}

void RuntimeQuadContext::evalDPdyCoarse(SpvmWord quadIdx, SpvmWord P) {
  syncQuadForResult(quadIdx, P);
  DerivativeRet *d = &dxdy_[P];

  SpvmValue *v0 = d->val[0];
  SpvmValue *v2 = d->val[2];

  if (v0 && v2) {
    SpvmValue *dy0 = createValue(v0->type, &sp_);
    valueSubF32(dy0, v2, v0);

    d->dy[0] = dy0;
    d->dy[1] = dy0;
    d->dy[2] = dy0;
    d->dy[3] = dy0;
  }
}

void RuntimeQuadContext::syncQuadForResult(SpvmWord quadIdx, SpvmWord P) {
  DerivativeRet *d = &dxdy_[P];
  for (SpvmWord i = 0; i < 4; i++) {
    if (i == quadIdx) {
      void *val = rts_[i].getRuntimeContext().results[P];
      storeDerivativeValue(&d->val[i], val);
      continue;
    }

    SpvmValue *val = nullptr;
    rts_[i].execContinue(P, &val);
    storeDerivativeValue(&d->val[i], val);
  }
}

void RuntimeQuadContext::storeDerivativeValue(SpvmValue **dst, void *val) {
  if (val) {
    *dst = createValue(((SpvmValue *) val)->type, &sp_);
    copyValue(*dst, (SpvmValue *) val);
  } else {
    *dst = nullptr;
  }
}

}
