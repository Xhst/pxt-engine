#version 450

layout(location = 0) in vec3 fragPosWorld;
layout(location = 1) in vec3 fragLightPos;

layout(location = 0) out float outColor;

void main() {
    // Store distance to light as 32 bit float value
    vec3 lightVec = fragPosWorld - fragLightPos;
    outColor = length(lightVec);
}