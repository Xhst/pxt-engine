#version 450
#extension GL_EXT_nonuniform_qualifier : enable

layout(constant_id = 0) const int MAX_LIGHTS = 10;

#define SHADOW_BIAS 0.005
#define SHADOW_BIAS_MIN 0.0005
#define SHADOW_OPACITY 0.4
#define PCF_RADIUS 0.001

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragPosWorld;
layout(location = 2) in vec3 fragNormalWorld;
layout(location = 3) in vec2 fragUV;
layout(location = 4) in mat3 fragTBN;

layout(location = 0) out vec4 outColor;

struct PointLight {
    vec4 position;  // .xyz = world position, .w = unused
    vec4 color;     // .xyz = RGB color, .w = intensity
};

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projectionMatrix;
    mat4 viewMatrix;
    mat4 inverseViewMatrix;
    vec4 ambientLightColor;
    PointLight pointLights[MAX_LIGHTS];
    int numLights;
} ubo;

layout(set = 1, binding = 0) uniform sampler2D textures[];
layout(set = 2, binding = 0) uniform samplerCube shadowCubeMap;

layout(push_constant) uniform Push {
    mat4 modelMatrix;
    mat4 normalMatrix;
    vec4 color;
    float specularIntensity;
    float shininess;
    int textureIndex;
    int normalMapIndex;
    int ambientOcclusionMapIndex;
    float tilingFactor;
} push;

/*
 * Extract and transform surface normal from the normal map.
 *
 * Samples the normal from a normal map, converts it from [0,1] to [-1,1],
 * and transforms it into world space using the TBN matrix.
 */
vec3 getSurfaceNormal(vec2 texCoords) {
    vec3 normalMapValue = texture(textures[push.normalMapIndex], texCoords).rgb;
    normalMapValue = normalMapValue * 2.0 - 1.0;
    return normalize(fragTBN * normalMapValue);
}

/*
 * Compute diffuse and specular lighting (Blinn-Phong model).
 *
 * Calculates the total diffuse and specular contributions from all active point lights.
 * Uses Blinn-Phong reflection for specular highlights.
 */
void computeLighting(vec3 surfaceNormal, vec3 viewDirection, out vec3 diffuseLight, out vec3 specularLight) {
    diffuseLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
    specularLight = vec3(0.0);

    for (int i = 0; i < ubo.numLights; i++) {
        PointLight light = ubo.pointLights[i];
        vec3 vectorToLight = light.position.xyz - fragPosWorld;
        float attenuation = 1.0 / dot(vectorToLight, vectorToLight);
        vec3 directionToLight = normalize(vectorToLight);
        float cosAngleIncidence = max(dot(surfaceNormal, directionToLight), 0.0);
        vec3 lightColor = light.color.xyz * light.color.w * attenuation;

        // Diffuse component
        diffuseLight += lightColor * cosAngleIncidence;

        // Specular component (Blinn-Phong)
        vec3 halfAngle = normalize(directionToLight + viewDirection);
        float blinnTerm = clamp(dot(surfaceNormal, halfAngle), 0.0, 1.0);
        blinnTerm = pow(blinnTerm, push.shininess);

        specularLight += lightColor * blinnTerm * push.specularIntensity;
    }
}

// Hash-based pseudo-random float generator
float rand(vec3 co) {
    return fract(sin(dot(co ,vec3(12.9898,78.233, 37.719))) * 43758.5453);
}

// Generates a normalized random direction on the sphere
vec3 randomDirection(int i, vec3 seed) {
    float xi1 = rand(seed + float(i) * vec3(1.0, 0.0, 0.0));
    float xi2 = rand(seed + float(i) * vec3(0.0, 1.0, 0.0));

    float theta = 2.0 * 3.14159265 * xi1;
    float phi = acos(2.0 * xi2 - 1.0);

    float x = sin(phi) * cos(theta);
    float y = sin(phi) * sin(theta);
    float z = cos(phi);

    return vec3(x, y, z);
}

/*
 * Computes the shadow factor for the fragment based on the distance to the light source
 *
 * Determines whether the fragment is in shadow by comparing the distance from
 * the fragment to the light against the sampled depth from the cube map.
 * A bias is applied to avoid shadow acne artifacts.
 */
float computeShadowFactor(vec3 surfaceNormal) {
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

/*
 * Applies ambient occlusion to the given color using the ambient occlusion map.
 */
void applyAmbientOcclusion(inout vec3 color, vec2 texCoords) {
    float ao = texture(textures[push.ambientOcclusionMapIndex], texCoords).r;
    color *= ao;
}

void main() {
    vec2 texCoords = fragUV * push.tilingFactor;

    vec3 surfaceNormal = getSurfaceNormal(texCoords);
    vec3 cameraPosWorld = ubo.inverseViewMatrix[3].xyz;
    vec3 viewDirection = normalize(cameraPosWorld - fragPosWorld);

    vec3 diffuseLight, specularLight;
    computeLighting(surfaceNormal, viewDirection, diffuseLight, specularLight);

    vec3 imageColor = texture(textures[push.textureIndex], texCoords).rgb;

    // we need to add control coefficients to regulate both terms (diffuse/specular)
    // for now we use fragColor for both which is ideal for metallic objects
    vec3 baseColor = (diffuseLight * fragColor + specularLight * fragColor) * imageColor;

    applyAmbientOcclusion(baseColor, texCoords);

    float shadow = computeShadowFactor(surfaceNormal);
    outColor = vec4(baseColor * shadow, 1.0);
}
