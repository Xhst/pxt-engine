#ifndef _RANDOM_
#define _RANDOM_

#include "math.glsl"

/**
 * @brief A pseudo-random number generator based on the Tiny Encryption Algorithm (TEA).
 * @param v0 The first 32-bit integer input.
 * @param v1 The second 32-bit integer input.
 * @return A pseudo-random 32-bit integer hash of the inputs.
 */
uint tea(uint v0, uint v1) {
    uint s0 = 0;
    for (uint n = 0; n < 4; n++) {
        s0 += 0x9e3779b9;
        v0 += ((v1 << 4) + 0xa341316c) ^ (v1 + s0) ^ ((v1 >> 5) + 0xc8013ea4);
        v1 += ((v0 << 4) + 0xad90777d) ^ (v0 + s0) ^ ((v0 >> 5) + 0x7e95761e);
    }
    return v0;
}

uint pcgHash(uint x) {
    uint state = x * 747796405u + 2891336453u;
    uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;

    return (word >> 22u) ^ word;
}

float randomFloat(inout uint seed) {
    uint result = pcgHash(seed);

    seed++;

    return float(result) * INV_UINT_MAX;
}



#endif