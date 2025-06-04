#version 460
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_GOOGLE_include_directive : require

#include "ubo/global_ubo.glsl"
#include "material/surface_normal.glsl"
#include "lighting/blinn_phong_lighting.glsl"
#include "lighting/shadow_map.glsl"

layout(location = 0) in vec3 fragPosWorld;
layout(location = 1) in vec3 fragNormalWorld;
layout(location = 2) in vec2 fragUV;
layout(location = 3) in mat3 fragTBN;

layout(location = 0) out vec4 outColor;

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
    int metallicMapIndex;
	int roughnessMapIndex;
    float tilingFactor;
} push;

/*
 * Applies ambient occlusion to the given color using the ambient occlusion map.
 */
void applyAmbientOcclusion(inout vec3 color, vec2 texCoords) {
    float ao = texture(textures[push.ambientOcclusionMapIndex], texCoords).r;
    color *= ao;
}

void main() {
    vec2 texCoords = fragUV * push.tilingFactor;

    vec3 surfaceNormal = calculateSurfaceNormal(textures[push.normalMapIndex], texCoords, fragTBN);

    vec3 cameraPosWorld = ubo.inverseViewMatrix[3].xyz;
    vec3 viewDirection = normalize(cameraPosWorld - fragPosWorld);

    vec3 diffuseLight, specularLight;
    computeBlinnPhongLighting(surfaceNormal, viewDirection, fragPosWorld, 
        push.shininess, push.specularIntensity, diffuseLight, specularLight);

    vec3 imageColor = texture(textures[push.textureIndex], texCoords).rgb;

    // we need to add control coefficients to regulate both terms (diffuse/specular)
    // for now we use fragColor for both which is ideal for metallic objects
    vec3 baseColor = (diffuseLight * push.color.rgb + specularLight * push.color.rgb) * imageColor;

    applyAmbientOcclusion(baseColor, texCoords);

    float shadow = computeShadowFactor(shadowCubeMap, surfaceNormal, fragPosWorld);

    outColor = vec4(baseColor * shadow, 1.0);
}
