#version 450

layout(binding = 0) uniform sampler2D texSampler;

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) flat in int inLod;
layout(location = 2) flat in ivec2 inFetchCoord;
layout(location = 3) in vec2 dPdx;
layout(location = 4) in vec2 dPdy;

layout(location = 0) out vec2 outQueryLod;
layout(location = 1) out ivec2 outTexSize;
layout(location = 2) out int outTexLevel;
layout(location = 3) out vec4 outColorFetch;
layout(location = 4) out vec4 outColorFetchOffset;
layout(location = 5) out vec4 outColorSample;
layout(location = 6) out vec4 outColorSampleGrad;

void main() {
    outQueryLod = textureQueryLod(texSampler, fragTexCoord);
    outTexSize = textureSize(texSampler, inLod);
    outTexLevel = textureQueryLevels(texSampler);

    outColorFetch = texelFetch(texSampler, inFetchCoord, inLod);
    outColorFetchOffset = texelFetchOffset(texSampler, inFetchCoord, inLod, ivec2(1, 1));

    outColorSample = texture(texSampler, fragTexCoord);
    outColorSampleGrad = textureGrad(texSampler, fragTexCoord, dPdx, dPdy);
}
