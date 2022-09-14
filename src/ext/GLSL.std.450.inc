/*
 * spvm
 * @author 	: keith@robot9.me
 *
 */

#include "GLSL.std.450.h"

#define REG_GLSL_OP_FUNC(op) ins[GLSLstd450##op] = (void*)func_GLSLstd450##op;
#define DEF_GLSL_OP_FUNC(op) DEF_OP_FUNC(GLSLstd450##op)

static RuntimeResult func_GLSLstd450_NotSupport(SpvmWord *pc, SpvmByte *sp, SpvmOpcode opcode, RuntimeContext *ctx) {
  LOGD("GLSL.std.450 Instruction not support, skip. opcode: %s, size: %d", spvmOpString(opcode.op), opcode.wordCount);
  SKIP_CURRENT_OPCODE();
  GO_NEXT
}

#define BEGIN_EXT_FUNCTION                                                            \
  SpvmTypeBase *resultType = SPVM_TYPE(READ_WORD());                                  \
  SpvmWord resultId = READ_WORD();                                                    \
  SKIP_WORD_N(2);                                                                     \
  SpvmValue *ret = CREATE_VALUE(resultType);                                          \

#define END_EXT_FUNCTION                                                              \
  ctx->results[resultId] = ret;                                                       \
  GO_NEXT                                                                             \

#define GLSL_MATH_ONE_IMPL(RetType, OpType, OP)                                       \
  BEGIN_EXT_FUNCTION                                                                  \
  SpvmValue *x = READ_OPERAND();                                                      \
  if (x->memberCnt > 0) {                                                             \
    for (SpvmWord i = 0; i < x->memberCnt; i++) {                                     \
      ret->VECTOR_##RetType(i) = OP(x->VECTOR_##OpType(i));                           \
    }                                                                                 \
  } else {                                                                            \
    ret->value.RetType = OP(x->value.OpType);                                         \
  }                                                                                   \
  END_EXT_FUNCTION                                                                    \

#define GLSL_MATH_TWO_IMPL(RetType, OpType, OP)                                       \
  BEGIN_EXT_FUNCTION                                                                  \
  SpvmValue *x = READ_OPERAND();                                                      \
  SpvmValue *y = READ_OPERAND();                                                      \
  if (x->memberCnt > 0) {                                                             \
    for (SpvmWord i = 0; i < x->memberCnt; i++) {                                     \
      ret->VECTOR_##RetType(i) = OP(x->VECTOR_##OpType(i), y->VECTOR_##OpType(i));    \
    }                                                                                 \
  } else {                                                                            \
    ret->value.RetType = OP(x->value.OpType, y->value.OpType);                        \
  }                                                                                   \
  END_EXT_FUNCTION                                                                    \

#define GLSL_MATH_THREE_IMPL(RetType, OpType, OP)                                     \
  BEGIN_EXT_FUNCTION                                                                  \
  SpvmValue *x = READ_OPERAND();                                                      \
  SpvmValue *y = READ_OPERAND();                                                      \
  SpvmValue *z = READ_OPERAND();                                                      \
  if (x->memberCnt > 0) {                                                             \
    for (SpvmWord i = 0; i < x->memberCnt; i++) {                                     \
      ret->VECTOR_##RetType(i) = OP(x->VECTOR_##OpType(i), y->VECTOR_##OpType(i),     \
                                    z->VECTOR_##OpType(i));                           \
    }                                                                                 \
  } else {                                                                            \
    ret->value.RetType = OP(x->value.OpType, y->value.OpType, z->value.OpType);       \
  }                                                                                   \
  END_EXT_FUNCTION                                                                    \

DEF_GLSL_OP_FUNC(Round) {
  GLSL_MATH_ONE_IMPL(f32, f32, round)
}

DEF_GLSL_OP_FUNC(RoundEven) {
  GLSL_MATH_ONE_IMPL(f32, f32, roundEven)
}

DEF_GLSL_OP_FUNC(Trunc) {
  GLSL_MATH_ONE_IMPL(f32, f32, trunc)
}

DEF_GLSL_OP_FUNC(FAbs) {
  GLSL_MATH_ONE_IMPL(f32, f32, fabs)
}

DEF_GLSL_OP_FUNC(SAbs) {
  GLSL_MATH_ONE_IMPL(i32, i32, abs)
}

DEF_GLSL_OP_FUNC(FSign) {
  GLSL_MATH_ONE_IMPL(f32, f32, fSign)
}

DEF_GLSL_OP_FUNC(SSign) {
  GLSL_MATH_ONE_IMPL(i32, i32, sSign)
}

DEF_GLSL_OP_FUNC(Floor) {
  GLSL_MATH_ONE_IMPL(f32, f32, floor)
}

DEF_GLSL_OP_FUNC(Ceil) {
  GLSL_MATH_ONE_IMPL(f32, f32, ceil)
}

DEF_GLSL_OP_FUNC(Fract) {
  GLSL_MATH_ONE_IMPL(f32, f32, fract)
}

DEF_GLSL_OP_FUNC(Radians) {
  GLSL_MATH_ONE_IMPL(f32, f32, radians)
}

DEF_GLSL_OP_FUNC(Degrees) {
  GLSL_MATH_ONE_IMPL(f32, f32, degrees)
}

DEF_GLSL_OP_FUNC(Sin) {
  GLSL_MATH_ONE_IMPL(f32, f32, sin)
}

DEF_GLSL_OP_FUNC(Cos) {
  GLSL_MATH_ONE_IMPL(f32, f32, cos)
}

DEF_GLSL_OP_FUNC(Tan) {
  GLSL_MATH_ONE_IMPL(f32, f32, tan)
}

DEF_GLSL_OP_FUNC(Asin) {
  GLSL_MATH_ONE_IMPL(f32, f32, asin)
}

DEF_GLSL_OP_FUNC(Acos) {
  GLSL_MATH_ONE_IMPL(f32, f32, acos)
}

DEF_GLSL_OP_FUNC(Atan) {
  GLSL_MATH_ONE_IMPL(f32, f32, atan)
}

DEF_GLSL_OP_FUNC(Sinh) {
  GLSL_MATH_ONE_IMPL(f32, f32, sinh)
}

DEF_GLSL_OP_FUNC(Cosh) {
  GLSL_MATH_ONE_IMPL(f32, f32, cosh)
}

DEF_GLSL_OP_FUNC(Tanh) {
  GLSL_MATH_ONE_IMPL(f32, f32, tanh)
}

DEF_GLSL_OP_FUNC(Asinh) {
  GLSL_MATH_ONE_IMPL(f32, f32, asinh)
}

DEF_GLSL_OP_FUNC(Acosh) {
  GLSL_MATH_ONE_IMPL(f32, f32, acosh)
}

DEF_GLSL_OP_FUNC(Atanh) {
  GLSL_MATH_ONE_IMPL(f32, f32, atanh)
}

DEF_GLSL_OP_FUNC(Atan2) {
  GLSL_MATH_TWO_IMPL(f32, f32, atan2)
}

DEF_GLSL_OP_FUNC(Pow) {
  GLSL_MATH_TWO_IMPL(f32, f32, pow)
}

DEF_GLSL_OP_FUNC(Exp) {
  GLSL_MATH_ONE_IMPL(f32, f32, exp)
}

DEF_GLSL_OP_FUNC(Log) {
  GLSL_MATH_ONE_IMPL(f32, f32, log)
}

DEF_GLSL_OP_FUNC(Exp2) {
  GLSL_MATH_ONE_IMPL(f32, f32, exp2)
}

DEF_GLSL_OP_FUNC(Log2) {
  GLSL_MATH_ONE_IMPL(f32, f32, log2)
}

DEF_GLSL_OP_FUNC(Sqrt) {
  GLSL_MATH_ONE_IMPL(f32, f32, sqrt)
}

DEF_GLSL_OP_FUNC(InverseSqrt) {
  GLSL_MATH_ONE_IMPL(f32, f32, inverseSqrt)
}

DEF_GLSL_OP_FUNC(Determinant) {
  BEGIN_EXT_FUNCTION
  SpvmValue *x = READ_OPERAND();
  matDeterminant(ret);
  END_EXT_FUNCTION
}

DEF_GLSL_OP_FUNC(MatrixInverse) {
  BEGIN_EXT_FUNCTION
  SpvmValue *x = READ_OPERAND();
  matInverse(ret, x);
  END_EXT_FUNCTION
}

DEF_GLSL_OP_FUNC(Modf) {
  BEGIN_EXT_FUNCTION
  SpvmValue *x = READ_OPERAND();
  SpvmPointer *pointer = (SpvmPointer *) ctx->results[READ_WORD()];
  SpvmValue *y = pointer->objPtr;
  if (x->memberCnt > 0) {
    for (SpvmWord i = 0; i < x->memberCnt; i++) {
      ret->VECTOR_f32(i) = modf(x->VECTOR_f32(i), &y->VECTOR_f32(i));
    }
  } else {
    ret->value.f32 = modf(x->value.f32, &y->value.f32);
  }
  END_EXT_FUNCTION
}

DEF_GLSL_OP_FUNC(ModfStruct) {
  BEGIN_EXT_FUNCTION
  SpvmValue *x = READ_OPERAND();
  ret->VECTOR_f32(0) = modf(x->value.f32, &ret->VECTOR_f32(1));
  END_EXT_FUNCTION
}

DEF_GLSL_OP_FUNC(FMin) {
  GLSL_MATH_TWO_IMPL(f32, f32, fMin)
}

DEF_GLSL_OP_FUNC(UMin) {
  GLSL_MATH_TWO_IMPL(u32, u32, uMin)
}

DEF_GLSL_OP_FUNC(SMin) {
  GLSL_MATH_TWO_IMPL(i32, i32, sMin)
}

DEF_GLSL_OP_FUNC(FMax) {
  GLSL_MATH_TWO_IMPL(f32, f32, fMax)
}

DEF_GLSL_OP_FUNC(UMax) {
  GLSL_MATH_TWO_IMPL(u32, u32, uMax)
}

DEF_GLSL_OP_FUNC(SMax) {
  GLSL_MATH_TWO_IMPL(i32, i32, sMax)
}

DEF_GLSL_OP_FUNC(FClamp) {
  GLSL_MATH_THREE_IMPL(f32, f32, fClamp);
}

DEF_GLSL_OP_FUNC(UClamp) {
  GLSL_MATH_THREE_IMPL(u32, u32, uClamp);
}

DEF_GLSL_OP_FUNC(SClamp) {
  GLSL_MATH_THREE_IMPL(i32, i32, sClamp);
}

DEF_GLSL_OP_FUNC(FMix) {
  GLSL_MATH_THREE_IMPL(f32, f32, fMix);
}

DEF_GLSL_OP_FUNC(Step) {
  GLSL_MATH_TWO_IMPL(f32, f32, step)
}

DEF_GLSL_OP_FUNC(SmoothStep) {
  GLSL_MATH_THREE_IMPL(f32, f32, smoothStep);
}

DEF_GLSL_OP_FUNC(Fma) {
  GLSL_MATH_THREE_IMPL(f32, f32, fma);
}

DEF_GLSL_OP_FUNC(Frexp) {
  BEGIN_EXT_FUNCTION
  SpvmValue *x = READ_OPERAND();
  SpvmPointer *pointer = (SpvmPointer *) ctx->results[READ_WORD()];
  SpvmValue *exp = pointer->objPtr;
  if (x->memberCnt > 0) {
    for (SpvmWord i = 0; i < x->memberCnt; i++) {
      ret->VECTOR_f32(i) = frexp(x->VECTOR_f32(i), (int *) &exp->VECTOR_i32(i));
    }
  } else {
    ret->value.f32 = frexp(x->value.f32, (int *) &exp->value.i32);
  }
  END_EXT_FUNCTION
}

DEF_GLSL_OP_FUNC(FrexpStruct) {
  BEGIN_EXT_FUNCTION
  SpvmValue *x = READ_OPERAND();
  ret->VECTOR_f32(0) = frexp(x->value.f32, (int *) &ret->VECTOR_i32(1));
  END_EXT_FUNCTION
}

DEF_GLSL_OP_FUNC(Ldexp) {
  BEGIN_EXT_FUNCTION
  SpvmValue *x = READ_OPERAND();
  SpvmValue *exp = READ_OPERAND();
  if (x->memberCnt > 0) {
    for (SpvmWord i = 0; i < x->memberCnt; i++) {
      ret->VECTOR_f32(i) = ldexp(x->VECTOR_f32(i), exp->VECTOR_i32(i));
    }
  } else {
    ret->value.f32 = ldexp(x->value.f32, exp->value.i32);
  }
  END_EXT_FUNCTION
}

DEF_GLSL_OP_FUNC(PackSnorm4x8) {
  BEGIN_EXT_FUNCTION
  SpvmValue *x = READ_OPERAND();
  union {
    SpvmI8 in[4];
    SpvmU32 out;
  } u;
  u.in[0] = round(fClamp(x->VECTOR_f32(0), -1.0f, 1.0f) * 127.0f);
  u.in[1] = round(fClamp(x->VECTOR_f32(1), -1.0f, 1.0f) * 127.0f);
  u.in[2] = round(fClamp(x->VECTOR_f32(2), -1.0f, 1.0f) * 127.0f);
  u.in[3] = round(fClamp(x->VECTOR_f32(3), -1.0f, 1.0f) * 127.0f);
  ret->value.u32 = u.out;
  END_EXT_FUNCTION
}

DEF_GLSL_OP_FUNC(PackUnorm4x8) {
  BEGIN_EXT_FUNCTION
  SpvmValue *x = READ_OPERAND();
  union {
    SpvmU8 in[4];
    SpvmU32 out;
  } u;
  u.in[0] = round(fClamp(x->VECTOR_f32(0), 0.0f, 1.0f) * 255.0f);
  u.in[1] = round(fClamp(x->VECTOR_f32(1), 0.0f, 1.0f) * 255.0f);
  u.in[2] = round(fClamp(x->VECTOR_f32(2), 0.0f, 1.0f) * 255.0f);
  u.in[3] = round(fClamp(x->VECTOR_f32(3), 0.0f, 1.0f) * 255.0f);
  ret->value.u32 = u.out;
  END_EXT_FUNCTION
}

DEF_GLSL_OP_FUNC(PackSnorm2x16) {
  BEGIN_EXT_FUNCTION
  SpvmValue *x = READ_OPERAND();
  union {
    SpvmI16 in[2];
    SpvmU32 out;
  } u;
  u.in[0] = round(fClamp(x->VECTOR_f32(0), -1.0f, 1.0f) * 32767.0f);
  u.in[1] = round(fClamp(x->VECTOR_f32(1), -1.0f, 1.0f) * 32767.0f);
  ret->value.u32 = u.out;
  END_EXT_FUNCTION
}

DEF_GLSL_OP_FUNC(PackUnorm2x16) {
  BEGIN_EXT_FUNCTION
  SpvmValue *x = READ_OPERAND();
  union {
    SpvmU16 in[2];
    SpvmU32 out;
  } u;
  u.in[0] = round(fClamp(x->VECTOR_f32(0), 0.0f, 1.0f) * 65535.0f);
  u.in[1] = round(fClamp(x->VECTOR_f32(1), 0.0f, 1.0f) * 65535.0f);
  ret->value.u32 = u.out;
  END_EXT_FUNCTION
}

DEF_GLSL_OP_FUNC(UnpackSnorm2x16) {
  BEGIN_EXT_FUNCTION
  SpvmValue *x = READ_OPERAND();
  union {
    SpvmU32 in;
    SpvmI16 out[2];
  } u;
  u.in = x->value.u32;
  ret->VECTOR_f32(0) = fClamp(u.out[0] / 32727.0f, -1.0f, 1.0f);
  ret->VECTOR_f32(1) = fClamp(u.out[1] / 32727.0f, -1.0f, 1.0f);
  END_EXT_FUNCTION
}

DEF_GLSL_OP_FUNC(UnpackUnorm2x16) {
  BEGIN_EXT_FUNCTION
  SpvmValue *x = READ_OPERAND();
  union {
    SpvmU32 in;
    SpvmU16 out[2];
  } u;
  u.in = x->value.u32;
  ret->VECTOR_f32(0) = u.out[0] / 65535.0f;
  ret->VECTOR_f32(1) = u.out[1] / 65535.0f;
  END_EXT_FUNCTION
}

DEF_GLSL_OP_FUNC(UnpackSnorm4x8) {
  BEGIN_EXT_FUNCTION
  SpvmValue *x = READ_OPERAND();
  union {
    SpvmU32 in;
    SpvmI8 out[4];
  } u;
  u.in = x->value.u32;
  ret->VECTOR_f32(0) = fClamp(u.out[0] / 127.0f, -1.0f, 1.0f);
  ret->VECTOR_f32(1) = fClamp(u.out[1] / 127.0f, -1.0f, 1.0f);
  ret->VECTOR_f32(2) = fClamp(u.out[2] / 127.0f, -1.0f, 1.0f);
  ret->VECTOR_f32(3) = fClamp(u.out[3] / 127.0f, -1.0f, 1.0f);
  END_EXT_FUNCTION
}

DEF_GLSL_OP_FUNC(UnpackUnorm4x8) {
  BEGIN_EXT_FUNCTION
  SpvmValue *x = READ_OPERAND();
  union {
    SpvmU32 in;
    SpvmU8 out[4];
  } u;
  u.in = x->value.u32;
  ret->VECTOR_f32(0) = u.out[0] / 255.0f;
  ret->VECTOR_f32(1) = u.out[1] / 255.0f;
  ret->VECTOR_f32(2) = u.out[2] / 255.0f;
  ret->VECTOR_f32(3) = u.out[3] / 255.0f;
  END_EXT_FUNCTION
}

DEF_GLSL_OP_FUNC(Length) {
  BEGIN_EXT_FUNCTION
  SpvmValue *x = READ_OPERAND();
  if (x->memberCnt > 0) {
    ret->value.f32 = 0;
    for (SpvmWord i = 0; i < x->memberCnt; i++) {
      ret->value.f32 += x->VECTOR_f32(i) * x->VECTOR_f32(i);
    }
    ret->value.f32 = sqrt(ret->value.f32);
  } else {
    ret->value.f32 = fabs(x->value.f32);
  }
  END_EXT_FUNCTION
}

DEF_GLSL_OP_FUNC(Distance) {
  BEGIN_EXT_FUNCTION
  SpvmValue *p0 = READ_OPERAND();
  SpvmValue *p1 = READ_OPERAND();
  if (p0->memberCnt > 0) {
    ret->value.f32 = 0;
    for (SpvmWord i = 0; i < p0->memberCnt; i++) {
      ret->value.f32 += (p0->VECTOR_f32(i) - p1->VECTOR_f32(i)) * (p0->VECTOR_f32(i) - p1->VECTOR_f32(i));
    }
    ret->value.f32 = sqrt(ret->value.f32);
  } else {
    ret->value.f32 = fabs(p0->value.f32 - p1->value.f32);
  }
  END_EXT_FUNCTION
}

DEF_GLSL_OP_FUNC(Cross) {
  BEGIN_EXT_FUNCTION
  SpvmValue *x = READ_OPERAND();
  SpvmValue *y = READ_OPERAND();
  ret->VECTOR_f32(0) = x->VECTOR_f32(1) * y->VECTOR_f32(2) - y->VECTOR_f32(1) * x->VECTOR_f32(2);
  ret->VECTOR_f32(1) = x->VECTOR_f32(2) * y->VECTOR_f32(0) - y->VECTOR_f32(2) * x->VECTOR_f32(0);
  ret->VECTOR_f32(2) = x->VECTOR_f32(0) * y->VECTOR_f32(1) - y->VECTOR_f32(0) * x->VECTOR_f32(1);
  END_EXT_FUNCTION
}

DEF_GLSL_OP_FUNC(Normalize) {
  BEGIN_EXT_FUNCTION
  SpvmValue *x = READ_OPERAND();
  if (x->memberCnt > 0) {
    SpvmF64 length = 0;
    for (SpvmWord i = 0; i < x->memberCnt; i++) {
      length += x->VECTOR_f32(i) * x->VECTOR_f32(i);
    }
    length = 1.0f / sqrt(length);
    for (SpvmWord i = 0; i < x->memberCnt; i++) {
      ret->VECTOR_f32(i) = x->VECTOR_f32(i) * length;
    }
  } else {
    ret->value.f32 = 1.0f;
  }
  END_EXT_FUNCTION
}

DEF_GLSL_OP_FUNC(FaceForward) {
  BEGIN_EXT_FUNCTION
  SpvmValue *N = READ_OPERAND();
  SpvmValue *I = READ_OPERAND();
  SpvmValue *Nref = READ_OPERAND();
  SpvmF64 dot = 0;
  for (SpvmWord i = 0; i < Nref->memberCnt; i++) {
    dot += Nref->VECTOR_f32(i) * I->VECTOR_f32(i);
  }
  for (SpvmWord i = 0; i < N->memberCnt; i++) {
    ret->VECTOR_f32(i) = dot < 0 ? N->VECTOR_f32(i) : -N->VECTOR_f32(i);
  }
  END_EXT_FUNCTION
}

DEF_GLSL_OP_FUNC(Reflect) {
  BEGIN_EXT_FUNCTION
  SpvmValue *I = READ_OPERAND();
  SpvmValue *N = READ_OPERAND();
  SpvmF64 dot = 0;
  for (SpvmWord i = 0; i < I->memberCnt; i++) {
    dot += I->VECTOR_f32(i) * N->VECTOR_f32(i);
  }
  dot *= 2;
  for (SpvmWord i = 0; i < N->memberCnt; i++) {
    ret->VECTOR_f32(i) = I->VECTOR_f32(i) - dot * N->VECTOR_f32(i);
  }
  END_EXT_FUNCTION
}

DEF_GLSL_OP_FUNC(Refract) {
  BEGIN_EXT_FUNCTION
  SpvmValue *I = READ_OPERAND();
  SpvmValue *N = READ_OPERAND();
  SpvmF64 eta = ((SpvmValue *) READ_OPERAND())->value.f32;
  SpvmF64 dot = 0;
  for (SpvmWord i = 0; i < I->memberCnt; i++) {
    dot += I->VECTOR_f32(i) * N->VECTOR_f32(i);
  }
  SpvmF64 k = 1.0f - eta * eta * (1.0f - dot * dot);
  for (SpvmWord i = 0; i < N->memberCnt; i++) {
    if (k < 0) {
      ret->VECTOR_f32(i) = 0;
    } else {
      ret->VECTOR_f32(i) = eta * I->VECTOR_f32(i) - (eta * dot + sqrt(k)) * N->VECTOR_f32(i);
    }
  }
  END_EXT_FUNCTION
}

DEF_GLSL_OP_FUNC(FindILsb) {
  GLSL_MATH_ONE_IMPL(i32, i32, findLsb)
}

DEF_GLSL_OP_FUNC(FindSMsb) {
  GLSL_MATH_ONE_IMPL(i32, i32, findMsb)
}

DEF_GLSL_OP_FUNC(FindUMsb) {
  GLSL_MATH_ONE_IMPL(i32, u32, findMsb)
}

DEF_GLSL_OP_FUNC(InterpolateAtCentroid) {
  BEGIN_EXT_FUNCTION
  // TODO
  END_EXT_FUNCTION
}

DEF_GLSL_OP_FUNC(InterpolateAtSample) {
  BEGIN_EXT_FUNCTION
  // TODO
  END_EXT_FUNCTION
}

DEF_GLSL_OP_FUNC(InterpolateAtOffset) {
  BEGIN_EXT_FUNCTION
  // TODO
  END_EXT_FUNCTION
}

DEF_GLSL_OP_FUNC(NMin) {
  GLSL_MATH_TWO_IMPL(f32, f32, nMin)
}

DEF_GLSL_OP_FUNC(NMax) {
  GLSL_MATH_TWO_IMPL(f32, f32, nMax)
}

DEF_GLSL_OP_FUNC(NClamp) {
  GLSL_MATH_THREE_IMPL(f32, f32, nClamp)
}

void registerGLSLstd450(void **ins, SpvmWord cnt) {
  for (SpvmWord i = 0; i < cnt; i++) {
    ins[i] = (void *) func_GLSLstd450_NotSupport;
  }

  REG_GLSL_OP_FUNC(Round)
  REG_GLSL_OP_FUNC(RoundEven)
  REG_GLSL_OP_FUNC(Trunc)
  REG_GLSL_OP_FUNC(FAbs)
  REG_GLSL_OP_FUNC(SAbs)
  REG_GLSL_OP_FUNC(FSign)
  REG_GLSL_OP_FUNC(SSign)
  REG_GLSL_OP_FUNC(Floor)
  REG_GLSL_OP_FUNC(Ceil)
  REG_GLSL_OP_FUNC(Fract)

  REG_GLSL_OP_FUNC(Radians)
  REG_GLSL_OP_FUNC(Degrees)
  REG_GLSL_OP_FUNC(Sin)
  REG_GLSL_OP_FUNC(Cos)
  REG_GLSL_OP_FUNC(Tan)
  REG_GLSL_OP_FUNC(Asin)
  REG_GLSL_OP_FUNC(Acos)
  REG_GLSL_OP_FUNC(Atan)
  REG_GLSL_OP_FUNC(Sinh)
  REG_GLSL_OP_FUNC(Cosh)
  REG_GLSL_OP_FUNC(Tanh)
  REG_GLSL_OP_FUNC(Asinh)
  REG_GLSL_OP_FUNC(Acosh)
  REG_GLSL_OP_FUNC(Atanh)
  REG_GLSL_OP_FUNC(Atan2)

  REG_GLSL_OP_FUNC(Pow)
  REG_GLSL_OP_FUNC(Exp)
  REG_GLSL_OP_FUNC(Log)
  REG_GLSL_OP_FUNC(Exp2)
  REG_GLSL_OP_FUNC(Log2)
  REG_GLSL_OP_FUNC(Sqrt)
  REG_GLSL_OP_FUNC(InverseSqrt)

  REG_GLSL_OP_FUNC(Determinant)
  REG_GLSL_OP_FUNC(MatrixInverse)

  REG_GLSL_OP_FUNC(Modf)
  REG_GLSL_OP_FUNC(ModfStruct)
  REG_GLSL_OP_FUNC(FMin)
  REG_GLSL_OP_FUNC(UMin)
  REG_GLSL_OP_FUNC(SMin)
  REG_GLSL_OP_FUNC(FMax)
  REG_GLSL_OP_FUNC(UMax)
  REG_GLSL_OP_FUNC(SMax)
  REG_GLSL_OP_FUNC(FClamp)
  REG_GLSL_OP_FUNC(UClamp)
  REG_GLSL_OP_FUNC(SClamp)
  REG_GLSL_OP_FUNC(FMix)
//  REG_GLSL_OP_FUNC(IMix)
  REG_GLSL_OP_FUNC(Step)
  REG_GLSL_OP_FUNC(SmoothStep)

  REG_GLSL_OP_FUNC(Fma)
  REG_GLSL_OP_FUNC(Frexp)
  REG_GLSL_OP_FUNC(FrexpStruct)
  REG_GLSL_OP_FUNC(Ldexp)

  REG_GLSL_OP_FUNC(PackSnorm4x8)

  REG_GLSL_OP_FUNC(PackUnorm4x8)
  REG_GLSL_OP_FUNC(PackSnorm2x16)
  REG_GLSL_OP_FUNC(PackUnorm2x16)
//  REG_GLSL_OP_FUNC(PackHalf2x16)
//  REG_GLSL_OP_FUNC(PackDouble2x32)
  REG_GLSL_OP_FUNC(UnpackSnorm2x16)
  REG_GLSL_OP_FUNC(UnpackUnorm2x16)
//  REG_GLSL_OP_FUNC(UnpackHalf2x16)
  REG_GLSL_OP_FUNC(UnpackSnorm4x8)
  REG_GLSL_OP_FUNC(UnpackUnorm4x8)
//  REG_GLSL_OP_FUNC(UnpackDouble2x32)

  REG_GLSL_OP_FUNC(Length)
  REG_GLSL_OP_FUNC(Distance)
  REG_GLSL_OP_FUNC(Cross)
  REG_GLSL_OP_FUNC(Normalize)
  REG_GLSL_OP_FUNC(FaceForward)
  REG_GLSL_OP_FUNC(Reflect)
  REG_GLSL_OP_FUNC(Refract)

  REG_GLSL_OP_FUNC(FindILsb)
  REG_GLSL_OP_FUNC(FindSMsb)
  REG_GLSL_OP_FUNC(FindUMsb)

  REG_GLSL_OP_FUNC(InterpolateAtCentroid)
  REG_GLSL_OP_FUNC(InterpolateAtSample)
  REG_GLSL_OP_FUNC(InterpolateAtOffset)

  REG_GLSL_OP_FUNC(NMin)
  REG_GLSL_OP_FUNC(NMax)
  REG_GLSL_OP_FUNC(NClamp)
}
