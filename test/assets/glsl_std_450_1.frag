#version 450

#extension GL_GOOGLE_include_directive : require
#include "assert.glsl"

#define PI 3.1415926535f

void main()
{
    ASSERT_BEGIN(64)

    // Determinant
    mat2 m1 = mat2(1.f, 0.f, 1.f, 1.f);
    float det = determinant(m1);
    ASSERT(det == 1.f)

    // MatrixInverse
    mat2 m2 = inverse(m1);
    ASSERT(m2 == mat2(1.f, 0.f, -1.f, 1.f));

    float f1 = 1.5f;
    float f2 = 0.f;
    uint u1 = 1;
    uint u2 = 2;
    int i1 = -1;
    int i2 = -2;

    // Modf
    ASSERT(modf(f1, f2) == 0.5f)
    ASSERT(f2 == 1.f)

    // ModfStruct TODO

    // FMin
    f1 = 1.5f;
    f2 = 0.f;
    ASSERT(min(f1, f2) == 0.f)

    // UMin
    ASSERT(min(u1, u2) == 1)

    // SMin
    ASSERT(min(i1, i2) == -2)

    // FMax
    ASSERT(max(f1, f2) == 1.5f)

    // UMax
    ASSERT(max(u1, u2) == 2)

    // SMax
    ASSERT(max(i1, i2) == -1)

    // FClamp
    ASSERT(clamp(f1, 0.f, 1.f) == 1.f)

    // UClamp
    ASSERT(clamp(u1, 0, 1) == 1)

    // SClamp
    ASSERT(clamp(i1, 0, 1) == 0)

    // FMix
    ASSERT(mix(f1, f2, 0.f) == f1)
    ASSERT(mix(f1, f2, 1.f) == f2)
    ASSERT(mix(f1, f2, 0.5f) == 0.75f)

    // IMix skip

    // Step
    ASSERT(step(1.4f, f1) == 1.f)
    ASSERT(step(1.6f, f1) == 0.f)

    // SmoothStep
    ASSERT(smoothstep(0.f, 3.f, f1) == 0.5f)

    // Fma
    ASSERT(fma(f1, 1.f, 0.5f) == 2.f)

    // Frexp
    int n = 0;
    ASSERT(frexp(8.f, n) == 0.5f)
    ASSERT(n == 4)

    // FrexpStruct TODO

    // Ldexp
    n = 4;
    ASSERT(ldexp(0.95f, n) == 15.2f)

    // PackSnorm4x8
    vec4 v1 = vec4(1.0f, 0.0f, -0.5f, -1.0f);
    u1 = 2176843903;
    ASSERT(packSnorm4x8(v1) == u1)

    // PackUnorm4x8
    vec4 v2 = vec4(1.0f, 0.5f, 0.0f, 1.0f);
    u2 = 4278223103;
    ASSERT(packUnorm4x8(v2) == u2)

    // PackSnorm2x16
    vec2 v3 = vec2(-0.5f, -0.7f);
    uint u3 = 2791817216;
    ASSERT(packSnorm2x16(v3) == u3)

    // PackUnorm2x16
    vec2 v4 = vec2(0.5f, 0.7f);
    uint u4 = 3006496768;
    ASSERT(packUnorm2x16(v4) == u4)

    // UnpackSnorm2x16
//    ASSERT(unpackSnorm2x16(u3) == v3)

    // UnpackUnorm2x16
//    ASSERT(unpackUnorm2x16(u4) == v4)

    // UnpackSnorm4x8
//    ASSERT(unpackSnorm4x8(u1) == v1)

    // UnpackUnorm4x8
//    ASSERT(unpackUnorm4x8(u2) == v2)

    // PackHalf2x16 TODO
    // UnpackHalf2x16 TODO
    // PackDouble2x32 TODO
    // UnpackDouble2x32 TODO

    ASSERT_END
}
