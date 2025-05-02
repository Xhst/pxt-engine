#ifndef __SURFACE_NORMAL__
#define __SURFACE_NORMAL__

/*
 * Extract and transform surface normal from the normal map.
 *
 * Samples the normal from a normal map, converts it from [0,1] to [-1,1],
 * and transforms it into world space using the TBN matrix.
 */
vec3 calculateSurfaceNormal(vec3 normalValue, mat3 TBN) {
    normalValue = normalValue * 2.0 - 1.0;

    return normalize(TBN * normalValue);
}

#endif