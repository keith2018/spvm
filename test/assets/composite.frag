#version 450

#extension GL_GOOGLE_include_directive : require
#include "assert.glsl"

void main()
{
    ASSERT_BEGIN(32)

    mat4 m1 = mat4(1.f, 2.f, 0.f, 0.f,
                   0.f, 1.f, 0.f, 0.f,
                   0.f, 0.f, 1.f, 0.f,
                   0.f, 0.f, 0.f, 1.f);

    mat4 m2 = transpose(m1);
    ASSERT(m2[0] == vec4(1.f, 0.f, 0.f, 0.f));
    ASSERT(m2[1] == vec4(2.f, 1.f, 0.f, 0.f));
    ASSERT(m2[2] == vec4(0.f, 0.f, 1.f, 0.f));
    ASSERT(m2[3] == vec4(0.f, 0.f, 0.f, 1.f));

    vec4 v1 = vec4(m1[0].yxz, 1.f);
    ASSERT(v1 == vec4(2.f, 1.f, 0.f, 1.f));

    ASSERT_END
}
