#ifndef _SHADOW_MAP_
#define _SHADOW_MAP_

#include "../ubo/global_ubo.glsl"

#define SHADOW_BIAS 0.005
#define SHADOW_BIAS_MIN 0.0005
#define SHADOW_OPACITY 0.4
#define PCF_RADIUS 0.003

/*
 * Computes the shadow factor for the fragment based on the distance to the light source
 *
 * Determines whether the fragment is in shadow by comparing the distance from
 * the fragment to the light against the sampled depth from the cube map.
 * A bias is applied to avoid shadow acne artifacts.
 */
float computeShadowFactor(samplerCube shadowCubeMap, vec3 surfaceNormal, vec3 fragPosWorld) {
    vec3 lightVec = fragPosWorld - ubo.pointLights[0].position.xyz;
    vec3 lightDir = normalize(lightVec);
    float bias = max(SHADOW_BIAS * (1.0 - dot(surfaceNormal, lightDir)), SHADOW_BIAS_MIN);
    float dist = length(lightVec);

    float shadow = 0.0;

    int totalSamples = 0;
    float sampledDist = 0.0;

    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            for (int z = -1; z <= 1; ++z) {
                vec3 offset = vec3(x, y, z);
                vec3 offsetDir = normalize(lightVec + (offset * PCF_RADIUS));
                sampledDist = texture(shadowCubeMap, offsetDir).r;
                if (dist > sampledDist + bias) {
                    shadow += 1.0;
                }
                totalSamples++;
            }
        }
    }

    float shadowFactor = 1.0 - (shadow / float(totalSamples));
    return mix(SHADOW_OPACITY, 1.0, shadowFactor); // soft blend
}

#endif