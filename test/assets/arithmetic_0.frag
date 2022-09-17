#version 450

#extension GL_GOOGLE_include_directive : require
#include "assert.glsl"

void main()
{
    ASSERT_BEGIN(32)

    // OpSNegate
    int a0 = 5;
    int outOpSNegate = -a0;
    ASSERT(outOpSNegate == -5);

    // OpFNegate
    float a1 = 5.f;
    float outOpFNegate = -a1;
    ASSERT(outOpFNegate == -5.f);

    // OpIAdd
    int a2 = 5;
    int outOpIAdd = a2 + 1;
    ASSERT(outOpIAdd == 6);

    // OpFAdd
    vec3 a3 = vec3(1.f, 1.f, 1.f);
    vec3 outOpFAdd = a3 + vec3(0.1f, 0.2f, 0.3f);
    ASSERT(outOpFAdd == vec3(1.1f, 1.2f, 1.3f));

    // OpISub
    ivec3 a4 = ivec3(5, 5, 5);
    ivec3 outOpISub = a4 - ivec3(4, 5, 6);
    ASSERT(outOpISub == ivec3(1, 0, -1));

    // OpFSub
    vec3 a5 = vec3(1.f, 1.f, 1.f);
    vec3 outOpFSub = a5 - vec3(0.1f, 0.2f, 0.3f);
    ASSERT(outOpFSub == vec3(0.9f, 0.8f, 0.7f));

    // OpIMul
    ivec3 a6 = ivec3(1, -2, 3);
    ivec3 outOpIMul = a6 * ivec3(4, 5, 6);
    ASSERT(outOpIMul == ivec3(4, -10, 18));

    // OpFMul
    vec3 a7 = vec3(1.f, -2.f, 1.f);
    vec3 outOpFMul = a7 * vec3(0.1f, 0.2f, 0.3f);
    ASSERT(outOpFMul == vec3(0.1f, -0.4f, 0.3f));

    // OpUDiv
    uvec3 a8 = uvec3(10, 20, 3);
    uvec3 outOpUDiv = a8 / uvec3(4, 5, 6);
    ASSERT(outOpUDiv == uvec3(2u, 4u, 0u));

    // OpSDiv
    ivec3 a9 = ivec3(10, -20, 30);
    ivec3 outOpSDiv = a9 / ivec3(4, 5, 6);
    ASSERT(outOpSDiv == ivec3(2, -4, 5));

    // OpFDiv
    vec3 a10 = vec3(10.f, -20.f, 30.f);
    vec3 outOpFDiv = a10 / vec3(4.f, 5.f, 6.f);
    ASSERT(outOpFDiv == vec3(2.5f, -4.f, 5.f));

    // OpUMod
    uvec3 a11 = uvec3(10, 21, 31);
    uvec3 outOpUMod = a11 % uvec3(4, 5, 6);
    ASSERT(outOpUMod == uvec3(2u, 1u, 1u));

    // OpSMod
    ivec3 a12 = ivec3(10, -21, 31);
    ivec3 outOpSMod = a12 % ivec3(4, 5, 6);
    ASSERT(outOpSMod == ivec3(2, 1, 1));

    // OpSRem TODO

    // OpFMod
    vec3 a13 = vec3(10.f, -21.f, 31.f);
    vec3 outOpFMod = mod(a13, vec3(4.f, 5.f, 6.f));
    ASSERT(outOpFMod == vec3(2.f, 1.f, 1.f));

    // OpFRem TODO

    ASSERT_END
}
