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
hitAttributeEXT vec2 attribs;

void main()
{
    // This shader is executed when a ray hits the closest geometry.

    // Store the hit distance in the payload
    payload.t = gl_HitTEXT;

    // --- Basic Lighting Example ---
    // In a real scenario, you would interpolate vertex attributes (like position, normal, UVs)
    // using the barycentric coordinates (attribs.barycentrics) and the vertex data
    // from your geometry buffers. For this basic example, we'll use dummy values.

    // Dummy surface normal (pointing upwards) - replace with interpolated normal
    vec3 surfaceNormal = vec3(0.0, 1.0, 0.0);
    // Ensure the normal is in world space and normalized
    // (In a real case, you'd transform the interpolated normal by the object's world matrix)
    surfaceNormal = normalize(surfaceNormal);

    // Simple diffuse lighting calculation for the first point light
    vec3 diffuseColor = vec3(0.0);
    if (ubo.numLights > 0) {
        PointLight light = ubo.pointLights[0]; // Use the first light

        // Calculate light direction from the hit point to the light source
        // gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT gives the hit point in world space
        vec3 hitPointWorld = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT;
        vec3 lightDir = normalize(light.position.xyz - hitPointWorld);

        // Calculate diffuse intensity (clamped to 0)
        float diff = max(dot(surfaceNormal, lightDir), 0.0);

        // Calculate diffuse color
        diffuseColor = light.color.rgb * diff;
    }

    // Combine ambient and diffuse light
    vec3 finalColor = ubo.ambientLightColor.rgb + diffuseColor;

    // You would typically sample textures here based on interpolated UV coordinates.
    // For this basic example, we'll just use the calculated lighting color.
    // If you had a texture, you might do something like:
    // vec2 uv = interpolate_uv_from_attribs(attribs.barycentrics);
    // vec4 texColor = texture(textures[0], uv); // Assuming texture index 0
    // finalColor *= texColor.rgb;

    // Store the final color in the payload
    payload.color = vec4(finalColor, 1.0);
}
