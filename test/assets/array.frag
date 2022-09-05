#version 450

#extension GL_GOOGLE_include_directive : require
#include "assert.glsl"

layout(set = 0, binding = 0) buffer TestBlock
{
    vec3 a;
    float[] b;
} tb;

void main()
{
    ASSERT_BEGIN(32)

    float arr[5] = float[5](3.4f, 4.2f, 5.0f, 5.2f, 1.1f);
    ASSERT(arr[0] == 3.4f);
    ASSERT(arr[1] == 4.2f);
    ASSERT(arr[2] == 5.0f);
    ASSERT(arr[3] == 5.2f);
    ASSERT(arr[4] == 1.1f);
    ASSERT(arr.length() == 5);

    ASSERT(tb.a == vec3(4.2f, 5.0f, 5.2f));

    int n = tb.b.length();
    ASSERT(n == 6);

    ASSERT(tb.b[0] == 1.2f);
    ASSERT(tb.b[1] == 1.0f);
    ASSERT(tb.b[2] == 1.3f);
    ASSERT(tb.b[3] == 0.2f);
    ASSERT(tb.b[4] == 0.3f);
    ASSERT(tb.b[5] == 0.5f);

    ASSERT_END
}
