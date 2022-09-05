#version 450
layout (location = 0) in vec3 vPosition;

layout(set = 0, binding = 0) uniform CameraBuffer
{
    vec4 data;
    mat4 viewProj;
} cameraData;

layout(push_constant) uniform constants
{
    vec4 data;
    mat4 renderMatrix;
} PushConstants;

void main()
{
    mat4 transformMatrix = (cameraData.viewProj * PushConstants.renderMatrix);
    gl_Position = transformMatrix * vec4(vPosition, 1.0f);
}
