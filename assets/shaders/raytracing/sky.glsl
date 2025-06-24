#ifndef _SKY_
#define _SKY_

#include "../ubo/global_ubo.glsl"

#define USE_SKY_AS_NEE_EMITTER 0

layout(set = 5, binding = 0) uniform samplerCube skyboxSampler;

vec3 getSkyRadiance(vec3 rayDir) {

	vec3 skyColor = texture(skyboxSampler, rayDir).rgb;

	return skyColor * ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
}

#endif