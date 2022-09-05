#version 450

#extension GL_GOOGLE_include_directive : require
#include "assert.glsl"

void main()
{
    ASSERT_BEGIN(32)

    // OpConvertFToU
    float f0 = 12.5f;
    uint outUint0 = uint(f0);
    ASSERT(outUint0 == 12u);

    // OpConvertFToS
    float f1 = -11.3f;
    int outInt0 = int(f1);
    ASSERT(outInt0 == -11);

    // OpConvertUToF
    float outFloat0 = float(outUint0);
    ASSERT(outFloat0 == 12.f);

    // OpConvertSToF
    float outFloat1 = float(outInt0);
    ASSERT(outFloat1 == -11.f);

    // OpBitcast
    uint u32 = 10;
    int outInt2 = int(u32);
    ASSERT(outInt2 == 10);

    // OpFConvert TODO

    ASSERT_END
}
