#ifndef _BRDF_NORMAL_DISTRIBUTION_
#define _BRDF_NORMAL_DISTRIBUTION_

#include "../../common/math.glsl"

/**
 * @brief Functions implementing specific specular Normal Distribution Functions (NDF, D Term).
 */

 /**
  * @brief Computes the GGX (Trowbridge-Reitz) Normal Distribution Function (NDF).
  *
  * @param roughness The roughness parameter of the microfacet surface.
  * @param NoH The dot product of the normal vector (N) and the half-vector (H).
  * @param h The half-vector (H).
  *
  * @return The value of the GGX NDF.
  *
  * @note Walter et al. 2007, "Microfacet Models for Refraction through Rough Surfaces"
  */
float D_GGX(float roughness, float NoH, const vec3 h) {
    float oneMinusNoHSquared = 1.0 - NoH * NoH;
    float a = NoH * roughness;
    float k = roughness / (oneMinusNoHSquared + a * a);
    float d = k * k * (1.0 / PI);

    return d;
}

/**
 * @brief Computes the Anisotropic GGX Normal Distribution Function (NDF).
 *
 * @param at Roughness in the tangent direction.
 * @param ab Roughness in the bitangent direction.
 * @param ToH The dot product of the tangent vector (T) and the half-vector (H).
 * @param BoH The dot product of the bitangent vector (B) and the half-vector (H).
 * @param NoH The dot product of the normal vector (N) and the half-vector (H).
 *
 * @return The value of the Anisotropic GGX NDF.
 *
 * @note Burley 2012, "Physically-Based Shading at Disney"
 */
float D_GGX_Anisotropic(float at, float ab, float ToH, float BoH, float NoH) {
    // The values at and ab are roughness^2, a2 is roughness^4
    // The dot product below computes then roughness^8 that does not fit
    // so a highp is used to avoid precision issues
    float a2 = at * ab;
    highp vec3 d = vec3(ab * ToH, at * BoH, a2 * NoH);
    highp float d2 = dot(d, d);
    float b2 = a2 / d2;

    return a2 * b2 * b2 * (1.0 / PI);
}

/**
 * @brief Computes the Charlie Normal Distribution Function (NDF), used for cloth.
 *
 * @param roughness The roughness parameter.
 * @param NoH The dot product of the normal vector (N) and the half-vector (H).
 *
 * @return The value of the Charlie NDF.
 *
 * @note Estevez and Kulla 2017, "Production Friendly Microfacet Sheen BRDF"
 */
float D_Charlie(float roughness, float NoH) {
    float invAlpha = 1.0 / roughness;
    float cos2h = NoH * NoH;
    float sin2h = max(1.0 - cos2h, 0.0078125);

    return (2.0 + invAlpha) * pow(sin2h, invAlpha * 0.5) / (2.0 * PI);
}

#endif 