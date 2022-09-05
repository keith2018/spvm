#version 450

#extension GL_GOOGLE_include_directive : require
#include "assert.glsl"

void main()
{
    ASSERT_BEGIN(64)

    bvec2 a = bvec2(true, false);
    bvec2 b = bvec2(false, false);
    bvec2 c = bvec2(true, true);
    bvec2 d = bvec2(false, true);

    // SpvOpAny
    ASSERT(any(a))
    ASSERT(!any(b))

    // SpvOpAll
    ASSERT(!all(a))
    ASSERT(!all(b))
    ASSERT(all(c))

    float fNan = intBitsToFloat(0x7ff80000);    // nan
    float fInf = 10.f / 0.f;                    // inf
    vec3 vf = vec3(1.f, fNan, fInf);

    // SpvOpIsNan
    ASSERT(isnan(fNan))
    ASSERT(isnan(vf) == bvec3(false, true, false))

    // SpvOpIsInf
    ASSERT(isinf(fInf))
    ASSERT(isinf(vf) == bvec3(false, false, true))

    // SpvOpLogicalEqual
    ASSERT(c == bvec2(true))

    // SpvOpLogicalNotEqual
    ASSERT(c != bvec2(false))

    // SpvOpLogicalOr
    bool b1 = true;
    bool b2 = false;
    ASSERT((b1 || b2))

    // SpvOpLogicalAnd
    ASSERT(!(b1 && b2))

    // SpvOpLogicalNot
    ASSERT(not(a) == d)
    ASSERT(not(b) == c)

    // SpvOpSelect
    ASSERT((b1 ? 0.1f : 0.2f) == 0.1f)
    ASSERT((b2 ? 0.1f : 0.2f) == 0.2f)


    int i1 = -1;
    int i2 = -2;
    uint u1 = 1;
    uint u2 = 2;

    // SpvOpIEqual
    ASSERT(i1 - 1 == i2)

    // SpvOpINotEqual
    ASSERT(i1 - 2 != i2)

    // SpvOpUGreaterThan
    ASSERT(u2 > u1)

    // SpvOpSGreaterThan
    ASSERT(i1 > i2)

    // SpvOpUGreaterThanEqual
    ASSERT(u2 >= u1)

    // SpvOpSGreaterThanEqual
    ASSERT(i1 >= i2)

    // SpvOpULessThan
    ASSERT(u1 < u2)

    // SpvOpSLessThan
    ASSERT(i2 < i1)

    // SpvOpULessThanEqual
    ASSERT(u1 <= u2)

    // SpvOpSLessThanEqual
    ASSERT(i2 <= i1)


    float f1 = 1.f;
    float f2 = 2.f;

    // SpvOpFOrdEqual
    ASSERT(f1 + 1.f == f2)

    // SpvOpFOrdNotEqual TODO

    // SpvOpFOrdLessThan
    ASSERT(f1 < f2)

    // SpvOpFOrdGreaterThan
    ASSERT(f2 > f1)

    // SpvOpFOrdLessThanEqual
    ASSERT(f1 <= f2)

    // SpvOpFOrdGreaterThanEqual
    ASSERT(f2 >= f1)

    // SpvOpFUnordEqual

    // SpvOpFUnordNotEqual
    ASSERT(f1 != f2)

    // SpvOpFUnordLessThan TODO
    // SpvOpFUnordGreaterThan TODO
    // SpvOpFUnordLessThanEqual TODO
    // SpvOpFUnordGreaterThanEqual TODO

    ASSERT_END
}
