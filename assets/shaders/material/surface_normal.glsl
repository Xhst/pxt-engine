#ifndef _SURFACE_NORMAL_
#define _SURFACE_NORMAL_

/*
 * Extract and transform surface normal from the normal map.
 *
 * Samples the normal from a normal map, converts it from [0,1] to [-1,1],
 * and transforms it into world space using the TBN matrix.
 */
vec3 calculateSurfaceNormal(sampler2D normalMap, vec2 textureCoords, mat3 TBN) {
    vec3 normalMapValue = texture(normalMap, textureCoords).rgb;

    normalMapValue = normalMapValue * 2.0 - 1.0;

    return normalize(TBN * normalMapValue);
}

#endif