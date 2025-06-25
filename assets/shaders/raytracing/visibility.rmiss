#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_GOOGLE_include_directive : require

#include "../common/payload.glsl"

layout(location = VisibilityPayloadLocation) rayPayloadInEXT bool p_isVisible;

void main() {
	p_isVisible = true; 
}