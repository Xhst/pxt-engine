#ifndef _MATH_
#define _MATH_

#define PI 3.14159265359

#define FLT_EPSILON 1e-5

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