#version 450

#extension GL_GOOGLE_include_directive : require
#include "assert.glsl"

#define PI 3.1415926535f

void main()
{
    ASSERT_BEGIN(64)

    // Length
    vec2 v2 = vec2(1.f, 0.f);
    vec3 v3 = vec3(1.f, 0.f, 0.f);
    vec4 v4 = vec4(1.f, 2.f, 3.f, 4.f);
    ASSERT(length(v2) == 1.f);
    ASSERT(length(v3) == 1.f);
    ASSERT(length(v4) == 5.47722578f);

    // Distance
    ASSERT(distance(v2, vec2(1.f, 0.f)) == 0.f);
    ASSERT(distance(v3, vec3(1.f, 0.f, 1.f)) == 1.f);
    ASSERT(distance(v4, vec4(1.f, 0.f, 0.f, 0.f)) == 5.38516474f);

    // Cross
    ASSERT(cross(v3, vec3(0.f, 1.f, 0.f)) ==  vec3(0.f, 0.f, 1.f));
    ASSERT(cross(vec3(0.f, 1.f, 0.f), v3) ==  vec3(0.f, 0.f, -1.f));

    // Normalize
    v3 = vec3(1.f, 2.f, 3.f);
    ASSERT(normalize(v2) == vec2(1.f, 0.f))
    ASSERT(normalize(v3) == vec3(0.267261237f, 0.534522474f, 0.801783681f))

    // FaceForward
    vec3 N = vec3(0.0f, 0.0f, 1.0f);
    vec3 I = vec3(1.0f, 0.0f, 1.0f);
    vec3 Nref = vec3(0.0f, 0.0f, 1.0f);
    vec3 F = faceforward(N, I, Nref);
    ASSERT(F == vec3(0.f, 0.f, -1.f));

    // Reflect
    vec2 A = vec2(1.0f, -1.0f);
    vec2 B = vec2(0.0f, 1.0f);
    vec2 C = reflect(A, B);
    ASSERT(C == vec2(1.f, 1.f))

    // Refract
    A = vec2(0.0f, -1.0f);
    B = vec2(0.0f, 1.0f);
    C = refract(A, B, 0.5f);
    ASSERT(C == vec2(0.f, -1.f))

    // FindILsb
    ASSERT(findLSB(0x00000001) == 0)
    ASSERT(findLSB(0x00000003) == 0)
    ASSERT(findLSB(0x00000002) == 1)
    ASSERT(findLSB(0x00010000) == 16)
    ASSERT(findLSB(0x7FFF0000) == 16)
    ASSERT(findLSB(0x7F000000) == 24)
    ASSERT(findLSB(0x7F00FF00) == 8)
    ASSERT(findLSB(0x00000000) == -1)

    // FindSMsb
    int a = 0x00000000;
    ASSERT(findMSB(a) == -1)
    a = 0x00000001;
    ASSERT(findMSB(a) == 0)
    a = 0x40000000;
    ASSERT(findMSB(a) == 30)
    a = -1;
    ASSERT(findMSB(a) == -1)

    ivec4 ia = ivec4(0x00000000);
    ASSERT(findMSB(ia) == ivec4(-1))
    ia = ivec4(0x00000001);
    ASSERT(findMSB(ia) == ivec4(0))
    ia = ivec4(0x40000000);
    ASSERT(findMSB(ia) == ivec4(30))

    // FindUMsb
    uint b = 0x00000000;
    ASSERT(findMSB(b) == -1)
    b = 0x00000001;
    ASSERT(findMSB(b) == 0)
    b = 0x40000000;
    ASSERT(findMSB(b) == 30)

    // InterpolateAtCentroid TODO
    // InterpolateAtSample TODO
    // InterpolateAtOffset TODO

    // NMin TODO
    // NMax TODO
    // NClamp TODO

    ASSERT_END
}
