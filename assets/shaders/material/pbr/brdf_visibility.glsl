#ifndef _BRDF_VISIBILITY_
#define _BRDF_VISIBILITY_

/**
 * @brief Functions implementing specific specular Visibility functions (Geometric Attenuation, V term).
 */

 /**
  * @brief Computes the Smith-GGX Correlated Visibility Function (V term).
  *
  * @param roughness The roughness parameter.
  * @param NoV The dot product of the normal vector (N) and the view vector (V).
  * @param NoL The dot product of the normal vector (N) and the light vector (L).
  *
  * @return The value of the Smith-GGX Correlated Visibility function.
  *
  * @note Heitz 2014, "Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs"
  */
float V_SmithGGXCorrelated(float roughness, float NoV, float NoL) {
    float a2 = roughness * roughness;
    float lambdaV = NoL * sqrt((NoV - a2 * NoV) * NoV + a2);
    float lambdaL = NoV * sqrt((NoL - a2 * NoL) * NoL + a2);
    float v = 0.5 / (lambdaV + lambdaL);

    return v;
}

/**
 * @brief Computes a fast approximation of the Smith-GGX Correlated Visibility Function (V term).
 *
 * @param roughness The roughness parameter.
 * @param NoV The dot product of the normal vector (N) and the view vector (V).
 * @param NoL The dot product of the normal vector (N) and the light vector (L).
 *
 * @return The value of the fast Smith-GGX Correlated Visibility function.
 *
 * @note Hammon 2017, "PBR Diffuse Lighting for GGX+Smith Microsurfaces"
 */
float V_SmithGGXCorrelated_Fast(float roughness, float NoV, float NoL) {
    float v = 0.5 / mix(2.0 * NoL * NoV, NoL + NoV, roughness);

    return v;
}

/**
 * @brief Computes the Anisotropic Smith-GGX Correlated Visibility Function (V term).
 *
 * @param at Roughness in the tangent direction.
 * @param ab Roughness in the bitangent direction.
 * @param ToV The dot product of the tangent vector (T) and the view vector (V).
 * @param BoV The dot product of the bitangent vector (B) and the view vector (V).
 * @param ToL The dot product of the tangent vector (T) and the light vector (L).
 * @param BoL The dot product of the bitangent vector (B) and the light vector (L).
 * @param NoV The dot product of the normal vector (N) and the view vector (V).
 * @param NoL The dot product of the normal vector (N) and the light vector (L).
 *
 * @return The value of the Anisotropic Smith-GGX Correlated Visibility function.
 *
 * @note Heitz 2014, "Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs"
 */
float V_SmithGGXCorrelated_Anisotropic(float at, float ab, float ToV, float BoV,
    float ToL, float BoL, float NoV, float NoL) {

    float lambdaV = NoL * length(vec3(at * ToV, ab * BoV, NoV));
    float lambdaL = NoV * length(vec3(at * ToL, ab * BoL, NoL));
    float v = 0.5 / (lambdaV + lambdaL);

    return v;
}

/**
 * @brief Computes the Kelemen Visibility Function (V term).
 *
 * @param LoH The dot product of the light vector (L) and the half-vector (H).
 *
 * @return The value of the Kelemen Visibility function.
 *
 * @note Kelemen 2001, "A Microfacet Based Coupled Specular-Matte BRDF Model with Importance Sampling"
 */
float V_Kelemen(float LoH) {
    return (0.25 / (LoH * LoH));
}

/**
 * @brief Computes the Neubelt Visibility Function (V term), used for cloth.
 *
 * @param NoV The dot product of the normal vector (N) and the view vector (V).
 * @param NoL The dot product of the normal vector (N) and the light vector (L).
 *
 * @return The value of the Neubelt Visibility function.
 *
 * @note Neubelt and Pettineo 2013, "Crafting a Next-gen Material Pipeline for The Order: 1886"
 */
float V_Neubelt(float NoV, float NoL) {
    return (1.0 / (4.0 * (NoL + NoV - NoL * NoV)));
}

#endif