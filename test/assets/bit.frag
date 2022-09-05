#version 450

#extension GL_GOOGLE_include_directive : require
#include "assert.glsl"

void main()
{
    ASSERT_BEGIN(64)

    // SpvOpShiftRightLogical
    uint ua = 0xfffffffe;
    ASSERT((ua >> 1) == 0x7fffffff)
    ASSERT((uvec2(ua) >> 1) == uvec2(0x7fffffff))

    // SpvOpShiftRightArithmetic
    int ia1 = -2; // 0xfffffffe
    ASSERT((ia1 >> 1) == 0xffffffff)
    ASSERT((ivec2(ia1) >> 1) == ivec2(0xffffffff))

    int ia2 = 0xff;
    ASSERT((ia2 >> 1) == 0x7f)
    ASSERT((ivec2(ia2) >> 1) == ivec2(0x7f))

    // SpvOpShiftLeftLogical
    ASSERT((ua << 1) == 0xfffffffc)
    ASSERT((uvec2(ua) << 1) == uvec2(0xfffffffc))

    ASSERT((ia1 << 1) == -4) // 0xfffffffc
    ASSERT((ivec2(ia1) << 1) == ivec2(-4))

    // SpvOpBitwiseOr
    ASSERT((ua | 0x1) == 0xffffffff)
    ASSERT((ua | 0) == 0xfffffffe)

    // SpvOpBitwiseXor
    ASSERT((ua ^ 0x1) == 0xffffffff)
    ASSERT((ua ^ 0xff) == 0xffffff01)

    // SpvOpBitwiseAnd
    ASSERT((ua & 0x1) == 0)
    ASSERT((ua & 0xff) == 0xfe)

    // SpvOpNot
    ASSERT((~ua) == 0x1)

    // SpvOpBitFieldInsert
    ASSERT(bitfieldInsert(0x00000000, 0xffffffff, 0, 32) == 0xffffffff)
    ASSERT(bitfieldInsert(0x00000000, 0xffffffff, 0, 31) == 0x7fffffff)
    ASSERT(bitfieldInsert(0x00000000, 0xffffffff, 0, 0) == 0x00000000)
    ASSERT(bitfieldInsert(0xff000000, 0x000000ff, 8, 8) == 0xff00ff00)
    ASSERT(bitfieldInsert(0xffff0000, 0xffff0000, 16, 16) == 0x00000000)
    ASSERT(bitfieldInsert(0x0000ffff, 0x0000ffff, 16, 16) == 0xffffffff)

    // SpvOpBitFieldSExtract
    ASSERT(bitfieldExtract(0x0f0f0f0f, 0, 32) == 0x0f0f0f0f)
    ASSERT(bitfieldExtract(0x000000ff, 1, 3) == 0x00000007)
    ASSERT(bitfieldExtract(0x0000ff00, 8, 8) == 0x000000ff)

    // SpvOpBitFieldUExtract
    ASSERT(bitfieldExtract(0xffffffffu, 0, 32) == 0xffffffff)
    ASSERT(bitfieldExtract(0xfffffff0u, 0, 5) == 0x00000010)

    // SpvOpBitReverse
    ASSERT(bitfieldReverse(0x55555555) == 0xAAAAAAAA)
    ASSERT(bitfieldReverse(0x0F0F0F0F) == 0xF0F0F0F0)

    // SpvOpBitCount
    ASSERT(bitCount(0x00000001) == 1)
    ASSERT(bitCount(0x00000003) == 2)
    ASSERT(bitCount(0x7fffffff) == 31)
    ASSERT(bitCount(0x00000000) == 0)

    ASSERT_END
}
