#version 460

#extension GL_EXT_ray_tracing : require
#extension GL_GOOGLE_include_directive : require

// Include the global UBO definition
#include "../ubo/global_ubo.glsl"

// Define the ray payload structure. Must match the raygen shader.
layout(location = 0) rayPayloadInEXT struct RayPayload {
    vec4 color; // The color accumulated along the ray
    float t;    // The hit distance (t-value)
} payload;

void main()
{
    // This shader is executed if the ray does not hit any geometry.
    // Set the color in the payload to a default value, e.g., the ambient light color.
    payload.color = ubo.ambientLightColor;
    payload.t = -1.0; // Indicate no hit
}