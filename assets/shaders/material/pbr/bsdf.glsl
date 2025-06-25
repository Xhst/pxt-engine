#ifndef _BSDF_
#define _BSDF_

#include "../../common/math.glsl"

struct SurfaceData {
    mat3 tbn;
    vec3 albedo;
    vec3 reflectance;
    float metalness;
    float roughness;
    float specularProbability;
};

/**
 * @brief Calculates the luminance of a given color.
 *
 * This function converts an RGB color to a single luminance value,
 * which can be useful for weighting or perceptual calculations.
 * Uses standard ITU-R BT.709 coefficients.
 *
 * @param color The input vec3 color in RGB space.
 * @return The luminance value of the color.
 */
float luminance(vec3 color) {
    return dot(color, vec3(0.2126, 0.7152, 0.0722));
}

/**
 * @brief Calculates the probability of sampling a specular lobe versus a diffuse lobe.
 *
 * This probability is used in hybrid sampling techniques to determine whether
 * to sample the specular or diffuse component of the BRDF. It's based on
 * the relative energy of the specular and diffuse reflections.
 *
 * @param albedo The base color of the material (diffuse component).
 * @param metalness The metalness of the material (0.0 for dielectric, 1.0 for metal).
 * @param reflectance The Fresnel F0 (reflectance at normal incidence) of the material.
 * @return A float representing the probability of choosing the specular path, clamped between 0.0 and 1.0.
 */
float calculateSpecularProbability(vec3 albedo, float metalness, vec3 reflectance) {
    const float weightSpecular = luminance(reflectance);
    const float weightDiffuse = lerp(luminance(albedo), 0.0, metalness);

    return min(weightSpecular / (weightDiffuse + weightSpecular + FLT_EPSILON), 1.0);
}

/**
 * @brief Calculates the base reflectance (F0) of a material.
 *
 * For dielectrics, F0 is a fixed value (0.04). For metals, F0 is equal to the albedo color.
 * This function interpolates between these two based on the metalness parameter.
 *
 * @param albedo The albedo color of the material.
 * @param metalness The metalness of the material (0.0 for dielectric, 1.0 for metal).
 * @return The Fresnel F0 (reflectance at normal incidence) as a vec3 color.
 */
vec3 calculateReflectance(vec3 albedo, float metalness) {
    // Default reflectance for dielectrics materials
    const vec3 F0_dielectric = vec3(0.04);

    // Reflectance for metals is equal to the albedo color
    return lerp(F0_dielectric, albedo, metalness);
}

/**
 * @brief Samples a direction from a cosine-weighted hemisphere.
 *
 * This function generates a random direction on the hemisphere above the surface normal,
 * with a probability distribution proportional to the cosine of the angle with the normal.
 * This is suitable for diffuse reflection.
 *
 * @param u A vec2 with two uniform random numbers in [0,1) for sampling.
 * @return A vec3 representing the sampled direction in tangent space, where Z is the normal.
 */
vec3 sampleCosineWeightedHemisphere(vec2 u) {
    const float r = sqrt(u.x);
    const float theta = TWO_PI * u.y;

    // disk sample
    const vec2 d = r * vec2(cos(theta), sin(theta));

    return vec3(d.x, d.y, sqrt(1.0 - d.x * d.x - d.y * d.y));
}

/**
 * @brief Evaluates the GGX Normal Distribution Function (NDF).
 *
 * The GGX NDF describes the distribution of microfacet normals on a surface.
 * Higher values indicate more microfacets aligned with the half-vector.
 *
 * @param NoH The cosine of the angle between the surface normal and the half-vector (N dot H).
 * @param roughness The roughness parameter of the material (0.0 for smooth, 1.0 for rough).
 * @return The value of the GGX NDF at the given angle and roughness.
 */
float D_GGX(float NoH, float roughness) {
    const float a2 = pow2(roughness);
    const float d = pow2(NoH) * (a2 - 1.0) + 1.0;

    return a2 / (PI * pow2(d));
}

/**
 * @brief Evaluates the Schlick-GGX Geometric Shadowing function for a single direction.
 *
 * This function approximates the shadowing and masking effects of microfacets for
 * a single direction (incoming or outgoing). It is a component of the Smith G function.
 *
 * @param cosTheta The cosine of the angle between the surface normal and the light/view direction.
 * @param roughness The roughness parameter of the material.
 * @return The value of the Schlick-GGX G1 term.
 */
float G_Schlick_GGX(float cosTheta, float roughness) {
    const float r = (pow2(roughness) + 1.0);
    const float k = pow2(r) / 8.0; // k for direct lighting

    return cosTheta / (cosTheta * (1.0 - k) + k);
}

/**
 * @brief Evaluates the Smith Geometric Shadowing function with the Schlick-GGX approximation.
 *
 * The Smith G function accounts for shadowing and masking of microfacets,
 * preventing light from reaching or leaving parts of the surface. It is the
 * product of G1 terms for incoming and outgoing directions.
 *
 * @param NoO The cosine of the angle between the surface normal and the outgoing direction.
 * @param NoI The cosine of the angle between the surface normal and the incoming direction.
 * @param roughness The roughness parameter of the material.
 * @return The value of the Smith G function.
 */
float G_Smith(float NoO, float NoI, float roughness) {
    return G_Schlick_GGX(NoO, roughness) * G_Schlick_GGX(NoI, roughness);
}

/**
 * @brief Evaluates the Schlick's approximation for the Fresnel equation.
 *
 * The Fresnel equation describes how the reflectance of light changes with the
 * angle of incidence. Schlick's approximation provides a good balance between
 * accuracy and performance.
 *
 * @param f0 The Fresnel reflectance at normal incidence (F0).
 * @param HoO The cosine of the angle between the half-vector and the outgoing direction.
 * @return The Fresnel term as a vec3 color.
 */
vec3 F_Schlick(vec3 f0, float HoO) {
    return f0 + (vec3(1.0) - f0) * pow5(1.0 - HoO);
}

/**
 * @brief Calculates the PDF for the GGX Normal Distribution Function.
 *
 * This is the probability density function for sampling a microfacet normal
 * based on the GGX distribution.
 *
 * @param NoH The cosine of the angle between the surface normal and the half-vector (N dot H).
 * @param roughness The roughness parameter of the material.
 * @return The PDF value for the GGX NDF.
 */
float pdfD_GGX(float NoH, float roughness) {
    return D_GGX(NoH, roughness) * NoH;
}

/**
 * @brief Calculates the PDF for a cosine-weighted hemisphere distribution.
 *
 * This is the probability density function for sampling directions on a hemisphere
 * with a cosine distribution, suitable for diffuse reflection.
 *
 * @param cosTheta The cosine of the angle between the sampled direction and the normal.
 * @return The PDF value for the cosine-weighted hemisphere.
 */
float pdfCosineWeightedHemisphere(float cosTheta) {
    return cosTheta * INV_PI;
}

/**
 * @brief Importance samples a microfacet normal based on the GGX distribution.
 *
 * This function generates a random half-vector that is distributed according to
 * the GGX NDF, which is efficient for sampling specular reflections.
 * The generated vector is in tangent space, where Z is the normal.
 *
 * @param r A vec2 with two uniform random numbers in [0,1) for sampling.
 * @param roughness The roughness parameter of the material.
 * @return A vec3 representing the importance sampled half-vector in tangent space.
 */
vec3 importanceSampleGGX(vec2 r, float roughness) {
    const float alpha2 = pow2(roughness);
    const float phi = TWO_PI * r.x;
    const float cosTheta = sqrt((1.0 - r.y) / (1.0 + (alpha2 - 1.0 + FLT_EPSILON) * r.y));
    const float sinTheta = sqrt(1.0 - pow2(cosTheta));

    return vec3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
}

/**
 * @brief Evaluates the PBR BRDF (Bidirectional Reflectance Distribution Function).
 *
 * This function combines the diffuse (Lambertian) and specular (GGX) components
 * of the BRDF to calculate the outgoing radiance for a given incoming light direction.
 *
 * @param surface A SurfaceData struct containing material properties (normal, albedo, roughness, metalness, reflectance).
 * @param outLightDir The outgoing (view) direction from the surface point.
 * @param inLightDir The incoming light direction towards the surface point.
 * @param halfVector The half-vector between the outgoing and incoming light directions.
 * @return The BRDF value as a vec3 color.
 */
vec3 evaluateBSDF(SurfaceData surface, vec3 outLightDir, vec3 inLightDir, vec3 halfVector) {
    const float NoH = cosThetaTangent(halfVector);
    const float NoI = cosThetaTangent(inLightDir);
    const float NoO = cosThetaTangent(outLightDir);

    const float HoO = cosTheta(halfVector, outLightDir);

    const float D = D_GGX(NoH, surface.roughness);
    const float G = G_Smith(NoO, NoI, surface.roughness);
    const vec3  F = F_Schlick(surface.reflectance, HoO);

    const vec3 kd = mix(vec3(1.0) - F, vec3(0.0), surface.metalness);

    const float specularDenominator = 4.0 * NoO * NoI + FLT_EPSILON;
    const vec3 specular = D * F * G / specularDenominator;

    const vec3 diffuse = surface.albedo * kd * INV_PI;

    return diffuse + specular;
}

/**
 * @brief Calculates the PDF (Probability Density Function) of the chosen BSDF sample.
 *
 * This function returns the probability of sampling a particular incoming light direction
 * for a hybrid diffuse-specular sampling strategy.
 *
 * @param surface A SurfaceData struct containing material properties.
 * @param outLightDir The outgoing (view) direction from the surface point.
 * @param inLightDir The incoming light direction towards the surface point.
 * @param halfVector The half-vector between the outgoing and incoming light directions.
 * @return The PDF value for the given sampled direction.
 */
float pdfBSDF(SurfaceData surface, vec3 outLightDir, vec3 inLightDir, vec3 halfVector) {
    const float NoH = cosThetaTangent(halfVector);
    const float NoI = cosThetaTangent(inLightDir);

    const float IoH = dot(inLightDir, halfVector);

    const float specularPdf = pdfD_GGX(NoH, surface.roughness) / max(4.0 * IoH, FLT_EPSILON);
    const float diffusePdf = pdfCosineWeightedHemisphere(NoI);

    return mix(diffusePdf, specularPdf, surface.specularProbability);
}

/**
 * @brief Samples a direction from the BSDF (Bidirectional Scattering Distribution Function).
 *
 * This function randomly chooses between sampling the specular or diffuse lobe
 * based on the calculated specular probability and returns a sampled incoming
 * light direction. It also outputs the PDF of the chosen sample and the
 * evaluated BRDF for that direction.
 *
 * @param surface A SurfaceData struct containing material properties.
 * @param outLightDir The outgoing (view) direction from the surface point.
 * @param inLightDir Output: The sampled incoming light direction.
 * @param pdf Output: The PDF of the sampled direction.
 * @param seed Inout: A seed for the random number generator, updated by the function.
 * @return The evaluated BRDF value for the sampled direction, weighted by cosine and inverse PDF.
 */
vec3 sampleBSDF(SurfaceData surface, vec3 outLightDir, out vec3 inLightDir, out float pdf, out bool isSpecular, inout uint seed) {
    // Half vector between the outgoing light direction and the incoming light direction
    vec3 halfVector;

    vec3 rand3 = randomVec3(seed);

    isSpecular = false;

    if (rand3.z < surface.specularProbability) {
        // Sample specular reflection
        halfVector = importanceSampleGGX(rand3.xy, surface.roughness);
        inLightDir = -reflect(outLightDir, halfVector);
        isSpecular = true;
    } else {
        // Sample diffuse reflection
        inLightDir = sampleCosineWeightedHemisphere(rand3.xy);
        halfVector = normalize(outLightDir + inLightDir);
    }

    const float cosTheta = cosThetaTangent(inLightDir);

    pdf = pdfBSDF(surface, outLightDir, inLightDir, halfVector);

    if (pdf < FLT_EPSILON) {
        return vec3(0.0);
    }

    const vec3 brdf = evaluateBSDF(surface, outLightDir, inLightDir, halfVector);

    return brdf * cosTheta / pdf;
}

#endif