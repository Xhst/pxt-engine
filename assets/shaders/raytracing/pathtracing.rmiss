#version 460

#extension GL_EXT_ray_tracing : require
#extension GL_GOOGLE_include_directive : require

// Include the global UBO definition
#include "../ubo/global_ubo.glsl"
#include "../common/payload.glsl"

layout(location = 0) rayPayloadInEXT PathTracePayload p_pathTrace;

vec3 getSkyColor(vec3 direction) {
    float t = 0.5 * (direction.y + 1.0);
    return mix(vec3(1.0, 1.0, 1.0), vec3(0.5, 0.7, 1.0), t) * 0.5; // Simple gradient sky
}

// skybox
layout(set = 5, binding = 0) uniform samplerCube skyboxSampler;

void main()
{
    //vec3 skyColor = vec3(0.0); getSkyColor(gl_WorldRayDirectionEXT);
    vec3 skyColor = texture(skyboxSampler, gl_WorldRayDirectionEXT).rgb;

    p_pathTrace.radiance += skyColor * ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
    // Mark the path as finished.
    p_pathTrace.done = true;
}