#version 460

#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_GOOGLE_include_directive : require

// Include the global UBO definition
#include "../ubo/global_ubo.glsl"

// Declare the textures binding.
// The descriptor_indexing extension allows using arrays of textures with a runtime-sized descriptor array.
layout(set = 2, binding = 0) uniform sampler2D textures[];

// Define the ray payload structure. Must match the raygen and miss shaders.
layout(location = 0) rayPayloadInEXT struct RayPayload {
    vec4 color; // The color accumulated along the ray
    float t;    // The hit distance (t-value)
} payload;

// Define the ray attributes structure.
// layout(location = 0) hitAttributeEXT is used to receive attributes from the intersection.
// For triangles, this implicitly receives barycentric coordinates.
hitAttributeEXT vec2 HitAttribs;

void main()
{
    // This shader is executed when a ray hits the closest geometry.

    // Store the hit distance in the payload
    payload.t = gl_HitTEXT;

    const vec3 barycentrics = vec3(1.0f - HitAttribs.x - HitAttribs.y, HitAttribs.x, HitAttribs.y);
    payload.color = vec4(barycentrics, 1.0);
}
