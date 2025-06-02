#version 460
#extension GL_GOOGLE_include_directive : require

#include "ubo/global_ubo.glsl"

layout(set = 1, binding = 0) uniform samplerCube skyboxSampler;

layout(location = 0) in vec3 inTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
    // Sample the cubemap using the interpolated texture coordinate (view direction)
    outColor = texture(skyboxSampler, inTexCoord);
}