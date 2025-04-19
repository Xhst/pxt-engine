#version 450

layout(constant_id = 0) const int MAX_LIGHTS = 10;

layout(location = 0) in vec3 fragPosWorld;
layout(location = 1) in vec3 fragLightPos;

layout(location = 0) out float outColor;

void main() {
    // Store distance to light as 32 bit float value
    vec3 lightVec = fragPosWorld - fragLightPos;
    outColor = length(lightVec);
}