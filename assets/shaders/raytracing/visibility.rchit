#version 460
#extension GL_EXT_ray_tracing : require

layout(location = 1) rayPayloadInEXT bool p_isVisible;

void main() {
	p_isVisible = false;
}