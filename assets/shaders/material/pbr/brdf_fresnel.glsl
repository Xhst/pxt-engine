#ifndef _BRDF_FRESNEL_
#define _BRDF_FRESNEL_

#include "../../common/math.glsl"

/**
 * @brief Functions implementing specific specular Fresnel functions (F term).
 */

 /**
  * @brief Computes the Schlick approximation of the Fresnel Function (F term).
  *
  * @param f0 The base Fresnel reflectance (at 0 degrees angle of incidence).
  * @param f90 The grazing Fresnel reflectance (at 90 degrees angle of incidence).
  * @param VoH The dot product of the view vector (V) and the half-vector (H).
  *
  * @return The Fresnel reflectance as a vec3.
  *
  * @note Schlick 1994, "An Inexpensive BRDF Model for Physically-Based Rendering"
  */
vec3 F_Schlick(const vec3 f0, float f90, float VoH) {
    return f0 + (f90 - f0) * pow5(1.0 - VoH);
}


/**
 * @brief Computes the Schlick approximation of the Fresnel Function (F term).
 *
 * @param f0 The base Fresnel reflectance (at 0 degrees angle of incidence).
 * @param VoH The dot product of the view vector (V) and the half-vector (H).
 *
 * @return The Fresnel reflectance as a vec3.
 */
vec3 F_Schlick(const vec3 f0, float VoH) {
    float f = pow(1.0 - VoH, 5.0);

    return f + f0 * (1.0 - f);
}

/**
 * @brief Computes the Schlick approximation of the Fresnel Function (F term) for a scalar f0.
 *
 * @param f0 The base Fresnel reflectance (at 0 degrees angle of incidence) as a scalar.
 * @param f90 The grazing Fresnel reflectance (at 90 degrees angle of incidence) as a scalar.
 * @param VoH The dot product of the view vector (V) and the half-vector (H).
 *
 * @return The Fresnel reflectance as a scalar.
 */
float F_Schlick(float f0, float f90, float VoH) {
    return f0 + (f90 - f0) * pow5(1.0 - VoH);
}

#endif