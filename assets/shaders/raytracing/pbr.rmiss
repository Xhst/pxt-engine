#version 460

#extension GL_EXT_ray_tracing : require
#extension GL_GOOGLE_include_directive : require

// Include the global UBO definition
#include "../ubo/global_ubo.glsl"

// Define the ray payload structure. Must match the raygen shader.
layout(location = 0) rayPayloadInEXT struct RayPayload {
    vec3 light;
    vec3 color;
    vec3 normal;
    float t; // The hit distance (t-value)
    uint seed;
} payload;

vec3 getSkyColor(vec3 direction) {
    float t = 0.5 * (direction.y + 1.0);
    return mix(vec3(1.0, 1.0, 1.0), vec3(0.5, 0.7, 1.0), t) * 0.5; // Simple gradient sky
}

// skybox
layout(set = 5, binding = 0) uniform samplerCube skyboxSampler;

void main()
{
    // This shader is executed if the ray does not hit any geometry.
    // Set the color in the payload to a default value, e.g., the ambient light color.
    //vec3 skyColor = getSkyColor(gl_WorldRayDirectionEXT);
    vec3 skyColor = texture(skyboxSampler, gl_WorldRayDirectionEXT).rgb;
    payload.light = skyColor;
    
    payload.t = -1.0; // Indicate no hit
}