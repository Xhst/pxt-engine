#version 460

#extension GL_EXT_ray_tracing : require
#extension GL_GOOGLE_include_directive : require

// Include the global UBO definition
#include "../ubo/global_ubo.glsl"

// Define the ray payload structure. Must match the raygen shader.
layout(location = 0) rayPayloadInEXT struct RayPayload {
    // Accumulated color and energy along the path.
    vec3 radiance;

    // The accumulated material reflectance/transmittance. It's multiplied at each bounce.
    vec3 throughput;

    // Current number of bounces. Used to limit path length and for Russian Roulette.
    int depth;

    // The origin of the ray in world space.
    vec3 origin; 

    // The direction of the ray in world space.
    vec3 direction;

    // A flag to signal that the path has been terminated (e.g., hit the sky, absorbed).
    bool done;

    // A seed for the random number generator, updated at each bounce.
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
    //vec3 skyColor = vec3(0.0); getSkyColor(gl_WorldRayDirectionEXT);
    vec3 skyColor = texture(skyboxSampler, gl_WorldRayDirectionEXT).rgb;

    // Add the environment's light contribution to the path's radiance.
    // This is modulated by the throughput, representing how much light energy
    // reached this point from the camera.
    payload.radiance += skyColor * payload.throughput;
    
    // Mark the path as finished.
    payload.done = true;
}