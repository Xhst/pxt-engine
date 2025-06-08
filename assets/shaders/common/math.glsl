#ifndef _MATH_
#define _MATH_

#define PI 3.14159265359

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

/**
 * Computes x^5 using only multiply operations.
 */
float pow5(float x) {
    float x2 = x * x;
    return x2 * x2 * x;
}

vec2 barycentricLerp(vec2 a, vec2 b, vec2 c, vec3 barycentrics) {
    return a * barycentrics.x + b * barycentrics.y + c * barycentrics.z;
}

vec3 barycentricLerp(vec3 a, vec3 b, vec3 c, vec3 barycentrics) {
    return a * barycentrics.x + b * barycentrics.y + c * barycentrics.z;
}

vec4 barycentricLerp(vec4 a, vec4 b, vec4 c, vec3 barycentrics) {
    return a * barycentrics.x + b * barycentrics.y + c * barycentrics.z;
}



#endif