#ifndef __GLOBAL_UBO__
#define __GLOBAL_UBO__

#include "../common/point_light.glsl"

layout(constant_id = 0) const int MAX_LIGHTS = 10;

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projectionMatrix;
    mat4 viewMatrix;
    mat4 inverseViewMatrix;
    vec4 ambientLightColor;
    PointLight pointLights[MAX_LIGHTS];
    int numLights;
} ubo;

#endif