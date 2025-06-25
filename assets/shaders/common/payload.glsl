#ifndef _PAYLOAD_
#define _PAYLOAD_

#define PathTracePayloadLocation  0
#define VisibilityPayloadLocation 1


struct PathTracePayload {
    // Accumulated color and energy along the path.
    vec3 radiance;

    // The accumulated material reflectance/transmittance. It's multiplied at each bounce.
    vec3 throughput;

    // Current number of bounces. Used to limit path length and for Russian Roulette.
    int depth;

    // The origin of the ray in world space.
    vec3 origin;

    // The direction of the ray in world space.
    vec3 direction;

    // A flag to signal that the path has been terminated (e.g., hit the sky, absorbed).
    bool done;

    // A seed for the random number generator, updated at each bounce.
    uint seed;

    bool isSpecularBounce;
};

#endif