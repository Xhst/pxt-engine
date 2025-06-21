#ifndef _MATH_
#define _MATH_

#define PI 3.14159265359
#define TWO_PI 6.28318530718
#define INV_PI 0.31830988618 
#define INV_TWO_PI 0.15915494309

#define INT_MAX 2147483647
#define INT_MIN -2147483648

#define UINT_MAX = 4294967295
#define INV_UINT_MAX 2.3283064365386963e-10 // 1/UINT_MAX

#define FLT_MAX 3.402823466e+38
#define FLT_MIN 1.175494351e-38

#define FLT_EPSILON 1e-5

/**
 * Clamps the value between 0 and 1.
 *
 * @param x The value to clamp.
 */
#define saturate(x) clamp(x, 0.0, 1.0)

/**
 * Linear interpolation between 'start' and 'end' by 't'.
 * t should be in the range [0, 1].
 * 
 * @param start The start value.
 * @param end The end value.
 * @param t The interpolation factor, clamped to [0, 1].
 * @return The interpolated value between 'start' and 'end'.
 */
#define lerp(start, end, t) mix(start, end, t)

/**
 * Computes x^2 as a single multiplication.
 */
float pow2(float x) {
    return x * x;
}

float pow4(float x) {
    float x2 = x * x;
    return x2 * x2;
}

/**
 * Computes x^5 using only multiply operations.
 */
float pow5(float x) {
    float x2 = x * x;
    return x2 * x2 * x;
}

float maxComponent(vec3 v) {
    return max(max(v.r, v.g), v.b);
}

vec2 barycentricLerp(vec2 a, vec2 b, vec2 c, vec2 barycentrics) {
    const float bZ = (1.0 - barycentrics.x - barycentrics.y);
    return bZ * a + barycentrics.x * b + barycentrics.y * c;
}

vec3 barycentricLerp(vec3 a, vec3 b, vec3 c, vec2 barycentrics) {
    const float bZ = (1.0 - barycentrics.x - barycentrics.y);
    return bZ * a + barycentrics.x * b + barycentrics.y * c;
}

vec4 barycentricLerp(vec4 a, vec4 b, vec4 c, vec2 barycentrics) {
    const float bZ = (1.0 - barycentrics.x - barycentrics.y);
    return bZ * a + barycentrics.x * b + barycentrics.y * c;
}




#endif