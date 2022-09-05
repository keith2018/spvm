#version 450

#extension GL_GOOGLE_include_directive : require
#include "assert.glsl"

float func1(float a, float b) {
    return a - b;
}

vec3 func2(vec3 a) {
    return a * 0.2f;
}

void main()
{
    ASSERT_BEGIN(32)

    vec3 inColor = vec3(0.2f, 0.3f, 0.4f);
    float outColor0 = func1(1.f - inColor.r, func2(inColor).g);
    ASSERT(outColor0 == 0.74f)

    vec4 outColor1 = vec4(func2(inColor), func1(1.0f, inColor.r));
    ASSERT(outColor1 == vec4(0.04f, 0.06f, 0.08f, 0.8f))

    ASSERT_END
}
