#version 450

layout (location = 0) in vec3 inColor;
layout (location = 0) out vec4 outDx;
layout (location = 1) out vec4 outDy;

void main()
{
    vec3 m = inColor.xzy;
    vec3 dx = dFdx(m);
    vec3 dy = dFdy(m);
    outDx = vec4(dx, 1.0f);
    outDy = vec4(dy, 1.0f);
}
