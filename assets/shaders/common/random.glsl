#ifndef _RANDOM_
#define _RANDOM_

#include "math.glsl"

uint pcgHash(uint x) {
    uint state = x * 747796405u + 2891336453u;
    uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
    
    return (word >> 22u) ^ word;
}

float randomFloat(uint seed) {
    seed = pcgHash(seed);
    
    return float(seed) * INV_UINT_MAX;
}

#endif