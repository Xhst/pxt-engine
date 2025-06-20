#ifndef _SHADOW_UBO_
#define _SHADOW_UBO_

#include "../lighting/point_light.glsl"

layout(constant_id = 0) const int MAX_LIGHTS = 10;

layout(set = 0, binding = 0) uniform ShadowUbo {
	mat4 projection;
	// this is a matrix that translates model coordinates to light coordinates
	mat4 lightOriginModel; // we could consider passing this as push constants in the future? (i think no, because we will have too many lights :(  )
	PointLight pointLights[MAX_LIGHTS];
	int numLights;
	uint time;
} ubo;

#endif