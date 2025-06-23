#version 460

#extension GL_EXT_ray_tracing : require
#extension GL_GOOGLE_include_directive : require

// Include the global UBO definition
#include "../ubo/global_ubo.glsl"
#include "../common/payload.glsl"
#include "sky.glsl"

layout(location = 0) rayPayloadInEXT PathTracePayload p_pathTrace;

void main()
{
    
    if (p_pathTrace.depth == 0) {
        p_pathTrace.radiance += getSkyRadiance(gl_WorldRayDirectionEXT) * p_pathTrace.throughput;
    }

    // Mark the path as finished.
    p_pathTrace.done = true;
}