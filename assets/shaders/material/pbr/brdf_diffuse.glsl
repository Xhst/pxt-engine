#ifndef _BRDF_DIFFUSE_
#define _BRDF_DIFFUSE_

#include "../../common/math.glsl"
#include "brdf_fresnel.glsl"

/**
 * @brief Functions implementing specific diffuse BRDF terms (Fd).
 */

 /**
  * @brief Computes the Lambertian diffuse term.

  * @return The value of the Lambertian diffuse term (1 / PI).
  */
float Fd_Lambert() {
    return 1.0 / PI;
}

/**
 * @brief Computes the Burley (Disney) diffuse term.
 *
 * @param roughness The roughness parameter.
 * @param NoV The dot product of the normal vector (N) and the view vector (V).
 * @param NoL The dot product of the normal vector (N) and the light vector (L).
 * @param LoH The dot product of the light vector (L) and the half-vector (H).
 *
 * @return The value of the Burley diffuse term.
 *
 * @note Burley 2012, "Physically-Based Shading at Disney"
 */
float Fd_Burley(float roughness, float NoV, float NoL, float LoH) {
    float f90 = 0.5 + 2.0 * roughness * LoH * LoH;
    float lightScatter = F_Schlick(1.0, f90, NoL);
    float viewScatter = F_Schlick(1.0, f90, NoV);

    return lightScatter * viewScatter * (1.0 / PI);
}

/**
 * @brief Computes an energy-conserving wrap diffuse term.
 *
 * @param NoL The dot product of the normal vector (N) and the light vector (L).
 * @param w The wrap-around parameter (controls how much light wraps around the surface).
 *
 * @return The value of the wrap diffuse term (excluding the 1/PI factor).
 */
float Fd_Wrap(float NoL, float w) {
    return saturate((NoL + w) / pow2(1.0 + w));
}

#endif 