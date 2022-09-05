#version 450

#extension GL_GOOGLE_include_directive : require
#include "assert.glsl"

void main()
{
    ASSERT_BEGIN(32)

    vec4 inVec = vec4(0.2f, 0.3f, 0.4f, 1.f);

    mat4 inMat = mat4(1.f, 1.f, 0.f, 0.f,
    0.f, 1.f, 0.f, 0.f,
    0.f, 0.f, 1.f, 0.f,
    0.f, 0.f, 0.f, 1.f);

    mat4 inMat2 = mat4(1.f, 2.f, 0.f, 0.f,
    0.f, 1.f, 0.f, 0.f,
    0.f, 0.f, 1.f, 0.f,
    0.f, 0.f, 0.f, 1.f);

    // OpVectorTimesScalar
    vec4 outOpVectorTimesScalar = inVec * 0.2f;
    ASSERT(outOpVectorTimesScalar == vec4(0.04f, 0.06f, 0.08f, 0.2f));

    // OpMatrixTimesScalar
    mat4 outOpMatrixTimesScalar = inMat * 0.2f;
    ASSERT(outOpMatrixTimesScalar[0] == vec4(0.2f, 0.2f, 0.f, 0.f));
    ASSERT(outOpMatrixTimesScalar[1] == vec4(0.f, 0.2f, 0.f, 0.f));
    ASSERT(outOpMatrixTimesScalar[2] == vec4(0.f, 0.f, 0.2f, 0.f));
    ASSERT(outOpMatrixTimesScalar[3] == vec4(0.f, 0.f, 0.f, 0.2f));

    // OpVectorTimesMatrix
    vec4 outOpVectorTimesMatrix = inVec * inMat;
    ASSERT(outOpVectorTimesMatrix == vec4(0.5f, 0.3f, 0.4f, 1.f));

    // OpMatrixTimesVector
    vec4 outOpMatrixTimesVector = inMat * inVec;
    ASSERT(outOpMatrixTimesVector == vec4(0.2f, 0.5f, 0.4f, 1.f));

    // OpMatrixTimesMatrix
    mat4 inMat22 = mat4(inMat2[0], inMat2[1], inMat2[3], inMat2[2]);
    mat4 outOpMatrixTimesMatrix = inMat * inMat22;
    ASSERT(outOpMatrixTimesMatrix[0] == vec4(1.f, 3.f, 0.f, 0.f));
    ASSERT(outOpMatrixTimesMatrix[1] == vec4(0.f, 1.f, 0.f, 0.f));
    ASSERT(outOpMatrixTimesMatrix[2] == vec4(0.f, 0.f, 0.f, 1.f));
    ASSERT(outOpMatrixTimesMatrix[3] == vec4(0.f, 0.f, 1.f, 0.f));

    vec4 v1 = vec4(1.f, 2.f, 3.f, 4.f);
    vec3 v2 = vec3(1.f, 2.f, 1.f);

    // SpvOpOuterProduct
    mat3x4 outerProduct = outerProduct(v1, v2);
    ASSERT(outerProduct[0] == vec4(1.f, 2.f, 3.f, 4.f));
    ASSERT(outerProduct[1] == vec4(2.f, 4.f, 6.f, 8.f));
    ASSERT(outerProduct[2] == vec4(1.f, 2.f, 3.f, 4.f));

    // SpvOpOuterProduct
    vec3 outCross = cross(v1.xyz, v2);
    ASSERT(outCross == vec3(-4.f, 2.f, 0.f));

    // SpvOpDot
    float outOpDot = dot(v1.xyz, v2);
    ASSERT(outOpDot == 8.f);

    // SpvOpIAddCarry
    uint uInteger1 = 0x00000001u;
    uint uInteger2 = 0x00000002u;
    uint uInteger3 = 0xFFFFFFFEu;
    uint carry;
    ASSERT(uaddCarry(uInteger1, uInteger3, carry) == 0xFFFFFFFFu);
    ASSERT(carry == 0u);
    ASSERT(uaddCarry(uInteger2, uInteger3, carry) == 0x00000001u);
    ASSERT(carry == 1u);

    // SpvOpISubBorrow
    uvec2 borrow;
    ASSERT(usubBorrow(uvec2(uInteger1, uInteger2), uvec2(uInteger2, uInteger1), borrow)
           == uvec2(0xFFFFFFFEu, 0x00000001u));
    ASSERT(borrow == uvec2(1u, 0u));

    // SpvOpUMulExtended
    uint ux = 0xffffffff;
    uint uy = 0x3;
    uint umsb = 0;
    uint ulsb = 0;
    umulExtended(ux, uy, umsb, ulsb);
    ASSERT(uvec2(umsb, ulsb) == uvec2(0x2, 0xfffffffd));

    uvec2 uvx = uvec2(0xffffffff, 0x1);
    uvec2 uvy = uvec2(0x3);
    uvec2 uvmsb = uvec2(0);
    uvec2 uvlsb = uvec2(0);
    umulExtended(uvx, uvy, uvmsb, uvlsb);
    ASSERT(uvmsb == uvec2(0x2, 0x0));
    ASSERT(uvlsb == uvec2(0xfffffffd, 0x3));

    // SpvOpSMulExtended
    int ix = 0xfffffff;
    int iy = 0xfff;
    int imsb = 0;
    int ilsb = 0;
    imulExtended(ix, iy, imsb, ilsb);
    ASSERT(ivec2(imsb, ilsb) == ivec2(0xff, 0xeffff001));

    ivec2 ivx = ivec2(0xfffffff, 0x1);
    ivec2 ivy = ivec2(0xfff);
    ivec2 ivmsb = ivec2(0);
    ivec2 ivlsb = ivec2(0);
    imulExtended(ivx, ivy, ivmsb, ivlsb);
    ASSERT(ivmsb == ivec2(0xff, 0x0));
    ASSERT(ivlsb == ivec2(0xeffff001, 0xfff));

    ASSERT_END
}
