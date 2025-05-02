#version 450
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_GOOGLE_include_directive : require

#include "ubo/global_ubo.glsl"
#include "material/surface_normal.glsl"

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragPosWorld;
layout(location = 2) in vec3 fragNormalWorld;
layout(location = 3) in vec2 fragUV;
layout(location = 4) in mat3 fragTBN;

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform sampler2D textures[];

layout(push_constant) uniform Push {
	mat4 modelMatrix;
	mat4 normalMatrix;
    vec4 color;
	int enableWireframe;
	int enableNormalsColor;
	int textureIndex;
	int normalMapIndex;
	int ambientOcclusionMapIndex;
	float tilingFactor;
} push;

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
        blinnTerm = pow(blinnTerm, 0);

        specularLight += lightColor * blinnTerm * 1;
    }
}


/*
 * Applies ambient occlusion to the given color using the ambient occlusion map.
 */
void applyAmbientOcclusion(inout vec3 color, vec2 texCoords) {
    float ao = texture(textures[push.ambientOcclusionMapIndex], texCoords).r;
    color *= ao;
}


void main() {
    if (push.enableWireframe == 1) {
        outColor = vec4(0.0, 1.0, 0.0, 1.0);
        return;
    }

    vec2 texCoords = fragUV * push.tilingFactor;

    vec3 surfaceNormal = normalize(fragNormalWorld);

    if (push.normalMapIndex != -1) {
        vec3 normalMapValue = texture(textures[push.normalMapIndex], texCoords).rgb;
        surfaceNormal = calculateSurfaceNormal(normalMapValue, fragTBN);
    }

    if (push.enableNormalsColor == 1) {
        outColor = vec4(surfaceNormal * 0.5 + 0.5, 1.0);
        return;
    }

    vec3 cameraPosWorld = ubo.inverseViewMatrix[3].xyz;
    vec3 viewDirection = normalize(cameraPosWorld - fragPosWorld);

    vec3 diffuseLight, specularLight;
    computeLighting(surfaceNormal, viewDirection, diffuseLight, specularLight);

    vec3 imageColor = vec3(1.0, 1.0, 1.0); // Default color
    if (push.textureIndex != -1) {
        imageColor = texture(textures[push.textureIndex], texCoords).rgb;
    }

    // we need to add control coefficients to regulate both terms (diffuse/specular)
    // for now we use fragColor for both which is ideal for metallic objects
    vec3 baseColor = (diffuseLight * fragColor + specularLight * fragColor) * imageColor;

    if (push.ambientOcclusionMapIndex != -1) {
        applyAmbientOcclusion(baseColor, texCoords);
    }

    outColor = vec4(baseColor, 1.0);
}
