#version 460

#extension GL_EXT_ray_tracing : require
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_debug_printf : enable

#include "../ubo/global_ubo.glsl"
#include "../common/payload.glsl"
#include "sky.glsl"

layout(location = PathTracePayloadLocation) rayPayloadInEXT PathTracePayload p_pathTrace;

void main()
{
#if USE_SKY_AS_NEE_EMITTER
    if (p_pathTrace.depth == 0) {
        p_pathTrace.radiance += getSkyRadiance(gl_WorldRayDirectionEXT) * p_pathTrace.throughput;
    }
#else
    p_pathTrace.radiance += getSkyRadiance(gl_WorldRayDirectionEXT) * p_pathTrace.throughput;
#endif

    // Mark the path as finished.
    p_pathTrace.done = true;
}