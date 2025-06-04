#ifndef _SURFACE_NORMAL_
#define _SURFACE_NORMAL_

/**
 * Calculate the Tangent-Bitangent-Normal (TBN) matrix.
 * This matrix is used to transform normals from tangent space to world space.
 * 
 * @param objectNormal The normal vector in object space
 * @param objectTangent The tangent vector in object space. (the w component is the handedness)
 * @param normalMatrix The normalMatrix (or modelViewMatrix) is used to convert the normal and tanget
 *                     vectors from object to world space.
 *
 * @return A 3x3 matrix that transforms normals from tangent space to world space.
 */
mat3 calculateTBN(vec4 objectNormal, vec4 objectTangent, mat3 normalMatrix) {

    vec3 worldNormal = normalize(normalMatrix * objectNormal.xyz);
    vec3 worldTanget = normalize(normalMatrix * objectTangent.xyz);
    float handedness = objectTangent.w;

    // normal
    vec3 N = worldNormal;

    // Gram–Schmidt process
    vec3 T = worldTanget - dot(worldNormal, worldTanget) * worldNormal;

    // bitangent
    vec3 B = cross(N, T) * handedness;

    return mat3(T, B, N);
}

/*
 * Extract and transform surface normal from the normal map.
 *
 * Samples the normal from a normal map, converts it from [0,1] to [-1,1],
 * and transforms it into world space using the TBN matrix.
 *
 * @param normalMap The sampler 2D normal map.
 * @param uv The texture coordinates.
 * @param TBN the Tangent-Bitangent-Normal (TBN) matrix.
 *
 * @return The surface normal in world space.
 */
vec3 calculateSurfaceNormal(sampler2D normalMap, vec2 uv, mat3 TBN) {
    vec3 normalMapValue = texture(normalMap, uv).rgb;

    // tangent space normal
    normalMapValue = normalMapValue * 2.0 - 1.0;

    return normalize(TBN * normalMapValue);
}

#endif