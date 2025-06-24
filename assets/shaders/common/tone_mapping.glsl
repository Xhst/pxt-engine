#ifndef _TONE_MAPPING_
#define _TONE_MAPPING_

vec3 reinhardToneMapping(vec3 color) {
	// Reinhard tone mapping
	vec3 mappedColor = color / (color + vec3(1.0));
	
	return mappedColor;
}

vec3 srgbToLinear(vec3 color) {
	// Convert sRGB to linear RGB
	return pow(color, vec3(2.2));
}

vec3 linearToSrgb(vec3 color) {
	// Convert linear RGB to sRGB
	return pow(color, vec3(1.0 / 2.2));
}

// ????
vec3 acesToneMapping(vec3 color) {
	// ACES Filmic tone mapping
	const vec3 a = vec3(0.59719, 0.07600, 0.02840);
	const vec3 b = vec3(0.35458, 0.90834, 0.13383);
	const vec3 c = vec3(0.04823, 0.01566, 0.00596);
	const vec3 d = vec3(0.00000, 0.00000, 0.00000);
	const vec3 e = vec3(1.00000, 1.00000, 1.00000);
	return color * (a * color + b) / (color * (c * color + d) + e);
}

#endif