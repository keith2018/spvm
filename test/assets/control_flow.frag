#version 450

#extension GL_GOOGLE_include_directive : require
#include "assert.glsl"

vec4 func_if(vec3 inColor) {
    vec4 outColor;
    if (inColor.r > 0.5f) {
        outColor = vec4(inColor, 1.f);
    } else {
        outColor = vec4(inColor, 0.f);
    }
    return outColor;
}

float func_switch(vec3 inColor) {
    float outColor = 0.f;
    switch (int(inColor.g)) {
        case 0: outColor = 0.1f; break;
        case 1: outColor = 0.2f; break;
        case 2: outColor = 0.3f; break;
        default :outColor = 0.4f; break;
    }
    return outColor;
}

void main()
{
    ASSERT_BEGIN(32)

    // if switch
    vec3 inColor = vec3(0.2f, 0.3f, 0.4f);
    vec4 outColor0 = func_if(inColor);
    float outColor1 = func_switch(inColor);
    ASSERT(outColor0 == vec4(0.2f, 0.3f, 0.4f, 0.f));
    ASSERT(outColor1 == 0.1f);

    inColor[0] = 0.6f;
    inColor[1] = 2.0f;
    outColor0 = func_if(inColor);
    outColor1 = func_switch(inColor);
    ASSERT(outColor0 == vec4(0.6f, 2.0f, 0.4f, 1.f));
    ASSERT(outColor1 == 0.3f);

    inColor[1] = 4.0f;
    outColor0 = func_if(inColor);
    outColor1 = func_switch(inColor);
    ASSERT(outColor0 == vec4(0.6f, 4.0f, 0.4f, 1.f));
    ASSERT(outColor1 == 0.4f);

    // phi
    vec3 rgb = vec3(0);
    if (inColor[0] >= 0.9f || inColor[1] >= 0.9f) {
        rgb = outColor0.yxz;
    } else {
        rgb = inColor;
    }
    ASSERT(rgb == vec3(4.0f, 0.6f, 0.4f))

    ASSERT_END
}
