#version 450

layout(set = 0, binding = 0) uniform InputData
{
    vec3      iResolution;           // viewport resolution (in pixels)
    float     iTime;                 // shader playback time (in seconds)
    float     iTimeDelta;            // render time (in seconds)
    int       iFrame;                // shader playback frame
    vec4      iMouse;                // mouse pixel coords. xy: current (if MLB down), zw: click
    vec4      iDate;                 // (year, month, day, time in seconds)
    float     iSampleRate;           // sound sample rate (i.e., 44100)
};

layout(set = 0, binding = 1) uniform sampler2D iChannel0;

layout (location = 0) in vec2 fragCoord;
layout (location = 0) out vec4 fragColor;

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    fragColor = texture(iChannel0, fragCoord);
}

void main()
{
    mainImage(fragColor, fragCoord);
}
