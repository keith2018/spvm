#version 450

#extension GL_GOOGLE_include_directive : require
#include "assert.glsl"

#define PI 3.1415926535f

void main()
{
    ASSERT_BEGIN(64)

    // Round
    float a = 0.f;
    ASSERT(round(a) == 0.f)
    a = 0.5f;
    ASSERT(round(a) == 1.f)
    a = 0.9f;
    ASSERT(round(a) == 1.f)
    a = -0.f;
    ASSERT(round(a) == 0.f)
    a = -0.5f;
    ASSERT(round(a) == -1.f)
    a = -0.9f;
    ASSERT(round(a) == -1.f)

    // RoundEven
    a = -1.5f;
    ASSERT(roundEven(a) == -2.f)
    a = 1.5f;
    ASSERT(roundEven(a) == 2.f)
    a = -2.5f;
    ASSERT(roundEven(a) == -2.f)
    a = 2.5f;
    ASSERT(roundEven(a) == 2.f)
    a = -2.6f;
    ASSERT(roundEven(a) == -3.f)
    a = 2.6f;
    ASSERT(roundEven(a) == 3.f)

    // Trunc
    a = 1.1f;
    ASSERT(trunc(a) == 1.f)
    a = -1.1f;
    ASSERT(trunc(a) == -1.f)
    a = 1.9f;
    ASSERT(trunc(a) == 1.f)
    a = -1.9f;
    ASSERT(trunc(a) == -1.f)

    // FAbs
    a = -1.f;
    ASSERT(abs(a) == 1.f)

    // SAbs
    ivec2 b = ivec2(-2);
    ASSERT(abs(b) == ivec2(2))

    // FSign
    a = 2.f;
    ASSERT(sign(a) == 1.f)
    a = 0.f;
    ASSERT(sign(a) == 0.f)
    a = -2.f;
    ASSERT(sign(a) == -1.f)

    // SSign
    b = ivec2(-2);
    ASSERT(sign(b) == ivec2(-1))
    b = ivec2(0);
    ASSERT(sign(b) == ivec2(0))
    b = ivec2(2);
    ASSERT(sign(b) == ivec2(1))

    // Floor
    a = 2.8f;
    ASSERT(floor(a) == 2.f)
    a = -2.8f;
    ASSERT(floor(a) == -3.f)

    // Ceil
    a = 2.8f;
    ASSERT(ceil(a) == 3.f)
    a = -2.8f;
    ASSERT(ceil(a) == -2.f)

    // Fract
    a = 2.8f;
    ASSERT(fract(a) == 0.8f)
    a = -2.8f;
    ASSERT(fract(a) == 0.2f)

    // Radians
    a = 180.f;
    ASSERT(radians(a) == PI)

    // Degrees
    a = PI;
    ASSERT(degrees(a) == 180.f)

    // Sin
    a = PI;
    ASSERT(sin(a) == 0.f)

    // Cos
    a = PI;
    ASSERT(cos(a) == -1.f)

    // Tan
    a = 0.f;
    ASSERT(tan(a) == 0.f)

    // Asin
    a = 0.f;
    ASSERT(asin(a) == 0.f)

    // Acos
    a = 1.f;
    ASSERT(acos(a) == 0.f)

    // Atan
    a = 0.f;
    ASSERT(atan(a) == 0.f)

    // Sinh
    a = 0.f;
    ASSERT(sinh(a) == 0.f)

    // Cosh
    a = 0.f;
    ASSERT(cosh(a) == 1.f)

    // Tanh
    a = 0.f;
    ASSERT(tanh(a) == 0.f)

    // Asinh
    a = 0.f;
    ASSERT(asinh(a) == 0.f)

    // Acosh
    a = 1.f;
    ASSERT(acosh(a) == 0.f)

    // Atanh
    a = 0.f;
    ASSERT(atanh(a) == 0.f)

    // Atan2
    a = 0.f;
    ASSERT(atan(a, 1.f) == 0.f)

    // Pow
    a = 2.f;
    ASSERT(pow(a, 3) == 8.f)

    // Exp
    a = 2.f;
    ASSERT(exp(a) == 7.38905609893f)

    // Log
    a = 7.38905609893f;
    ASSERT(log(a) == 2.f)

    // Exp2
    a = 2.f;
    ASSERT(exp2(a) == 4.f)

    // Log2
    a = 8.f;
    ASSERT(log2(a) == 3.f)

    // Sqrt
    a = 16.f;
    ASSERT(sqrt(a) == 4.f)

    // InverseSqrt
    a = 16.f;
    ASSERT(inversesqrt(a) == 0.25f)

    ASSERT_END
}
