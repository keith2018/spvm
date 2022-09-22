/*
 * spvm
 * @author 	: keith@robot9.me
 *
 */

#pragma once

#include <math.h>
#include <float.h>
#include "spvm.h"
#include "logger.h"

namespace SPVM {

const static SpvmF32 SPVM_PI = 3.14159265358979323846;
const static SpvmF32 SPVM_PI_DIV_180 = SPVM_PI / 180.0f;
const static SpvmF32 SPVM_180_DIV_PI = 180.0f / SPVM_PI;
const static SpvmF32 FLT_EPS = 0.000001f;

inline SpvmU32 bitMask(int bits) {
  return bits >= 32 ? ~0 : (0x1 << bits) - 1;
}

inline bool fEqual(SpvmF32 a, SpvmF32 b) {
  bool ret = fabs(a - b) <= FLT_EPS;
  return ret;
}

inline SpvmU32 bitInsert(SpvmU32 base, SpvmU32 insert, SpvmI32 offset, SpvmI32 count) {
  SpvmU32 mask = bitMask(count) << offset;
  return (base & ~mask) | ((insert << offset) & mask);
}

inline SpvmI32 bitExtractSigned(SpvmI32 value, SpvmI32 offset, SpvmI32 count) {
  return (value >> offset) & bitMask(count);
}

inline SpvmU32 bitExtractUnsigned(SpvmU32 value, SpvmI32 offset, SpvmI32 count) {
  return (value >> offset) & bitMask(count);
}

inline SpvmU32 bitCount(SpvmU32 value) {
#if defined(__GNUC__)
  return value == 0 ? 0 : __builtin_popcount(value);
#else
  SpvmU32 ret = 0;
  while (value) {
    value = (value & (value - 1));
    ret++;
  }
  return ret;
#endif
}

inline SpvmU32 bitReverse(SpvmU32 num) {
  SpvmU32 ret = 0;
  for (SpvmWord i = 0; i < 32; i++) {
    ret = (ret << 1) | (num & 0x1);
    num >>= 1;
  }
  return ret;
}

inline SpvmF32 fract(SpvmF32 x) {
  return x - floor(x);
}

inline SpvmF32 roundEven(SpvmF32 x) {
  int intVal = (int) (x);
  SpvmF32 intPart = (SpvmF32) (intVal);
  SpvmF32 fractPart = fract(x);

  if (fractPart > 0.5f || fractPart < 0.5f) {
    return round(x);
  } else if ((intVal % 2) == 0) {
    return intPart;
  } else if (x <= 0.f) {
    return intPart - 1.f;
  } else {
    return intPart + 1.f;
  }
}

inline SpvmF32 fSign(SpvmF32 x) {
  if (x > 0) return 1.0f;
  if (x < 0) return -1.0f;
  return 0;
}

inline SpvmI32 sSign(SpvmI32 x) {
  if (x > 0) return 1;
  if (x < 0) return -1;
  return 0;
}

inline SpvmI32 sRem(SpvmI32 x, SpvmI32 y) {
  if (y == 0) {
    return 0;
  }
  return x % y;
}

inline SpvmI32 sMod(SpvmI32 x, SpvmI32 y) {
  if (y == 0) {
    return 0;
  }
  SpvmI32 ret = x % y;
  if (sSign(x) != sSign(y)) {
    ret += y;
  }
  return ret;
}

inline SpvmF32 fRem(SpvmF32 x, SpvmF32 y) {
  return fmod(x, y);
}

inline SpvmF32 fMod(SpvmF32 x, SpvmF32 y) {
  return x - y * floor(x / y);
}

inline SpvmF32 fPow(SpvmF32 x, SpvmF32 y) {
  return pow(x, y);
}

inline SpvmF32 fAsin(SpvmF32 x) {
  return asin(x);
}

inline SpvmF32 fAcos(SpvmF32 x) {
  return acos(x);
}

inline SpvmF32 fAcosh(SpvmF32 x) {
  return acosh(x);
}

inline SpvmF32 fAtanh(SpvmF32 x) {
  return atanh(x);
}

inline SpvmF32 fAtan2(SpvmF32 y, SpvmF32 x) {
  return atan2(y, x);
}

inline SpvmF32 fLog(SpvmF32 x) {
  return log(x);
}

inline SpvmF32 fLog2(SpvmF32 x) {
  return log2(x);
}

inline SpvmF32 fSqrt(SpvmF32 x) {
  return sqrt(x);
}

inline SpvmF32 radians(SpvmF32 x) {
  return x * SPVM_PI_DIV_180;
}

inline SpvmF32 degrees(SpvmF32 x) {
  return x * SPVM_180_DIV_PI;
}

inline SpvmF32 inverseSqrt(SpvmF32 x) {
  return 1.f / sqrt(x);
}

inline SpvmF32 fMin(SpvmF32 a, SpvmF32 b) {
  return a > b ? b : a;
}

inline SpvmI32 sMin(SpvmI32 a, SpvmI32 b) {
  return a > b ? b : a;
}

inline SpvmU32 uMin(SpvmU32 a, SpvmU32 b) {
  return a > b ? b : a;
}

inline SpvmF32 fMax(SpvmF32 a, SpvmF32 b) {
  return a > b ? a : b;
}

inline SpvmI32 sMax(SpvmI32 a, SpvmI32 b) {
  return a > b ? a : b;
}

inline SpvmU32 uMax(SpvmU32 a, SpvmU32 b) {
  return a > b ? a : b;
}

inline SpvmF32 matDeterminant(SpvmValue * x) {
#define M_x(col, row) x->MATRIX_f32(col, row)
  switch (x->memberCnt) {
    case 2: {
      return M_x(0, 0) * M_x(1, 1) - M_x(1, 0) * M_x(0, 1);
    }
    case 3: {
      return M_x(0, 0) * (M_x(1, 1) * M_x(2, 2) - M_x(2, 1) * M_x(1, 2))
          - M_x(1, 0) * (M_x(0, 1) * M_x(2, 2) - M_x(2, 1) * M_x(0, 2))
          + M_x(2, 0) * (M_x(0, 1) * M_x(1, 2) - M_x(1, 1) * M_x(0, 2));
    }
    case 4: {
      SpvmF32 m2233 = M_x(2, 2) * M_x(3, 3) - M_x(3, 2) * M_x(2, 3);
      SpvmF32 m2133 = M_x(2, 1) * M_x(3, 3) - M_x(3, 1) * M_x(2, 3);
      SpvmF32 m2132 = M_x(2, 1) * M_x(3, 2) - M_x(3, 1) * M_x(2, 2);
      SpvmF32 m2033 = M_x(2, 0) * M_x(3, 3) - M_x(3, 0) * M_x(2, 3);
      SpvmF32 m2032 = M_x(2, 0) * M_x(3, 2) - M_x(3, 0) * M_x(2, 2);
      SpvmF32 m2031 = M_x(2, 0) * M_x(3, 1) - M_x(3, 0) * M_x(2, 1);

      return M_x(0, 0) * (M_x(1, 1) * m2233 - M_x(1, 2) * m2133 + M_x(1, 3) * m2132)
          - M_x(0, 1) * (M_x(1, 0) * m2233 - M_x(1, 2) * m2033 + M_x(1, 3) * m2032)
          + M_x(0, 2) * (M_x(1, 0) * m2133 - M_x(1, 1) * m2033 + M_x(1, 3) * m2031)
          - M_x(0, 3) * (M_x(1, 0) * m2132 - M_x(1, 1) * m2032 + M_x(1, 2) * m2031);
    }
    default:
      break;
  }
  return 0.f;
}

inline void matInverse(SpvmValue * ret, SpvmValue * x) {
#define M_ret(col, row) ret->MATRIX_f32(col, row)
#define M_x(col, row) x->MATRIX_f32(col, row)
  SpvmF32 invDet = matDeterminant(x);
  switch (x->memberCnt) {
    case 2: {
      M_ret(0, 0) = +M_x(1, 1) * invDet;
      M_ret(0, 1) = -M_x(0, 1) * invDet;
      M_ret(1, 0) = -M_x(1, 0) * invDet;
      M_ret(1, 1) = +M_x(0, 0) * invDet;
      break;
    }
    case 3: {
      M_ret(0, 0) = +(M_x(1, 1) * M_x(2, 2) - M_x(2, 1) * M_x(1, 2)) * invDet;
      M_ret(0, 1) = -(M_x(0, 1) * M_x(2, 2) - M_x(2, 1) * M_x(0, 2)) * invDet;
      M_ret(0, 2) = +(M_x(0, 1) * M_x(1, 2) - M_x(1, 1) * M_x(0, 2)) * invDet;
      M_ret(1, 0) = -(M_x(1, 0) * M_x(2, 2) - M_x(2, 0) * M_x(1, 2)) * invDet;
      M_ret(1, 1) = +(M_x(0, 0) * M_x(2, 2) - M_x(2, 0) * M_x(0, 2)) * invDet;
      M_ret(1, 2) = -(M_x(0, 0) * M_x(1, 2) - M_x(1, 0) * M_x(0, 2)) * invDet;
      M_ret(2, 0) = +(M_x(1, 0) * M_x(2, 1) - M_x(2, 0) * M_x(1, 1)) * invDet;
      M_ret(2, 1) = -(M_x(0, 0) * M_x(2, 1) - M_x(2, 0) * M_x(0, 1)) * invDet;
      M_ret(2, 2) = +(M_x(0, 0) * M_x(1, 1) - M_x(1, 0) * M_x(0, 1)) * invDet;
      break;
    }
    case 4: {
      SpvmF32 c00 = M_x(2, 2) * M_x(3, 3) - M_x(3, 2) * M_x(2, 3);
      SpvmF32 c02 = M_x(1, 2) * M_x(3, 3) - M_x(3, 2) * M_x(1, 3);
      SpvmF32 c03 = M_x(1, 2) * M_x(2, 3) - M_x(2, 2) * M_x(1, 3);

      SpvmF32 c04 = M_x(2, 1) * M_x(3, 3) - M_x(3, 1) * M_x(2, 3);
      SpvmF32 c06 = M_x(1, 1) * M_x(3, 3) - M_x(3, 1) * M_x(1, 3);
      SpvmF32 c07 = M_x(1, 1) * M_x(2, 3) - M_x(2, 1) * M_x(1, 3);

      SpvmF32 c08 = M_x(2, 1) * M_x(3, 2) - M_x(3, 1) * M_x(2, 2);
      SpvmF32 c10 = M_x(1, 1) * M_x(3, 2) - M_x(3, 1) * M_x(1, 2);
      SpvmF32 c11 = M_x(1, 1) * M_x(2, 2) - M_x(2, 1) * M_x(1, 2);

      SpvmF32 c12 = M_x(2, 0) * M_x(3, 3) - M_x(3, 0) * M_x(2, 3);
      SpvmF32 c14 = M_x(1, 0) * M_x(3, 3) - M_x(3, 0) * M_x(1, 3);
      SpvmF32 c15 = M_x(1, 0) * M_x(2, 3) - M_x(2, 0) * M_x(1, 3);

      SpvmF32 c16 = M_x(2, 0) * M_x(3, 2) - M_x(3, 0) * M_x(2, 2);
      SpvmF32 c18 = M_x(1, 0) * M_x(3, 2) - M_x(3, 0) * M_x(1, 2);
      SpvmF32 c19 = M_x(1, 0) * M_x(2, 2) - M_x(2, 0) * M_x(1, 2);

      SpvmF32 c20 = M_x(2, 0) * M_x(3, 1) - M_x(3, 0) * M_x(2, 1);
      SpvmF32 c22 = M_x(1, 0) * M_x(3, 1) - M_x(3, 0) * M_x(1, 1);
      SpvmF32 c23 = M_x(1, 0) * M_x(2, 1) - M_x(2, 0) * M_x(1, 1);

      M_ret(0, 0) = +(M_x(1, 1) * c00 - M_x(1, 2) * c04 + M_x(1, 3) * c08) * invDet;
      M_ret(0, 1) = -(M_x(0, 1) * c00 - M_x(0, 2) * c04 + M_x(0, 3) * c08) * invDet;
      M_ret(0, 2) = +(M_x(0, 1) * c02 - M_x(0, 2) * c06 + M_x(0, 3) * c10) * invDet;
      M_ret(0, 3) = -(M_x(0, 1) * c03 - M_x(0, 2) * c07 + M_x(0, 3) * c11) * invDet;

      M_ret(1, 0) = -(M_x(1, 0) * c00 - M_x(1, 2) * c12 + M_x(1, 3) * c16) * invDet;
      M_ret(1, 1) = +(M_x(0, 0) * c00 - M_x(0, 2) * c12 + M_x(0, 3) * c16) * invDet;
      M_ret(1, 2) = -(M_x(0, 0) * c02 - M_x(0, 2) * c14 + M_x(0, 3) * c18) * invDet;
      M_ret(1, 3) = +(M_x(0, 0) * c03 - M_x(0, 2) * c15 + M_x(0, 3) * c19) * invDet;

      M_ret(2, 0) = +(M_x(1, 0) * c04 - M_x(1, 1) * c12 + M_x(1, 3) * c20) * invDet;
      M_ret(2, 1) = -(M_x(0, 0) * c04 - M_x(0, 1) * c12 + M_x(0, 3) * c20) * invDet;
      M_ret(2, 2) = +(M_x(0, 0) * c06 - M_x(0, 1) * c14 + M_x(0, 3) * c22) * invDet;
      M_ret(2, 3) = -(M_x(0, 0) * c07 - M_x(0, 1) * c15 + M_x(0, 3) * c23) * invDet;

      M_ret(3, 0) = -(M_x(1, 0) * c08 - M_x(1, 1) * c16 + M_x(1, 2) * c20) * invDet;
      M_ret(3, 1) = +(M_x(0, 0) * c08 - M_x(0, 1) * c16 + M_x(0, 2) * c20) * invDet;
      M_ret(3, 2) = -(M_x(0, 0) * c10 - M_x(0, 1) * c18 + M_x(0, 2) * c22) * invDet;
      M_ret(3, 3) = +(M_x(0, 0) * c11 - M_x(0, 1) * c19 + M_x(0, 2) * c23) * invDet;
      break;
    }
    default:
      break;
  }
}

inline SpvmF32 fClamp(SpvmF32 d, SpvmF32 min, SpvmF32 max) {
  return fMin(fMax(d, min), max);
}

inline SpvmU32 uClamp(SpvmU32 d, SpvmU32 min, SpvmU32 max) {
  return uMin(uMax(d, min), max);
}

inline SpvmI32 sClamp(SpvmI32 d, SpvmI32 min, SpvmI32 max) {
  return sMin(sMax(d, min), max);
}

inline SpvmF32 fMix(SpvmF32 x, SpvmF32 y, SpvmF32 a) {
  return x * (1.0f - a) + y * a;
}

inline SpvmF32 step(SpvmF32 edge, SpvmF32 x) {
  return x >= edge ? 1.0f : 0.0f;
}

inline SpvmF32 smoothStep(SpvmF32 edge0, SpvmF32 edge1, SpvmF32 x) {
  SpvmF32 t = fClamp((x - edge0) / (edge1 - edge0), 0, 1);
  return t * t * (3.0f - (2.0f * t));
}

inline SpvmF32 fma(SpvmF32 a, SpvmF32 b, SpvmF32 c) {
  return a * b + c;
}

inline SpvmI32 findLsb(SpvmI32 x) {
  int i;
  int mask;
  int res = -1;
  for (i = 0; i < 32; i++) {
    mask = 1 << i;
    if (x & mask) {
      res = i;
      break;
    }
  }
  return res;
}

inline SpvmI32 findMsb(SpvmI32 x) {
  int i;
  int mask;
  int res = -1;
  if (x < 0) x = ~x;
  for (i = 0; i < 32; i++) {
    mask = 0x80000000 >> i;
    if (x & mask) {
      res = 31 - i;
      break;
    }
  }
  return res;
}

inline SpvmF32 nMin(SpvmF32 a, SpvmF32 b) {
  if (isnan(a)) {
    return b;
  } else if (isnan(b)) {
    return a;
  } else {
    return a > b ? b : a;
  }
}

inline SpvmF32 nMax(SpvmF32 a, SpvmF32 b) {
  if (isnan(a)) {
    return b;
  } else if (isnan(b)) {
    return a;
  } else {
    return a < b ? b : a;
  }
}

inline SpvmF32 nClamp(SpvmF32 d, SpvmF32 min, SpvmF32 max) {
  return nMin(nMax(d, min), max);
}

inline SpvmVec4 vec4FAdd(SpvmVec4 a, SpvmVec4 b) {
  SpvmVec4 ret;
  for (SpvmWord i = 0; i < 4; i++) {
    ret.elem[i].f32 = a.elem[i].f32 + b.elem[i].f32;
  }
  return ret;
}

inline SpvmVec4 vec4FMul(SpvmVec4 a, SpvmF32 b) {
  SpvmVec4 ret;
  for (SpvmWord i = 0; i < 4; i++) {
    ret.elem[i].f32 = a.elem[i].f32 * b;
  }
  return ret;
}

inline SpvmF32 vec4FDot(SpvmVec4 a, SpvmVec4 b) {
  SpvmF32 ret = 0.f;
  for (SpvmWord i = 0; i < 4; i++) {
    ret += a.elem[i].f32 * b.elem[i].f32;
  }
  return ret;
}

}