#ifndef _BLINN_PHONG_LIGHTING_
#define _BLINN_PHONG_LIGHTING_

#include "../common/math.glsl"
#include "../ubo/global_ubo.glsl"

/*
 * Compute diffuse and specular lighting (Blinn-Phong model).
 *
 * Calculates the total diffuse and specular contributions from all active point lights.
 * Uses Blinn-Phong reflection for specular highlights.
 */
void computeBlinnPhongLighting(vec3 surfaceNormal, vec3 viewDirection, vec3 worldPosition,
	float shininess, float specularIntensity, out vec3 diffuseLight, out vec3 specularLight) {

    diffuseLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
    specularLight = vec3(0.0);

    for (int i = 0; i < ubo.numLights; i++) {
        PointLight light = ubo.pointLights[i];
        vec3 vectorToLight = light.position.xyz - worldPosition;
        float attenuation = 1.0 / dot(vectorToLight, vectorToLight);
        vec3 directionToLight = normalize(vectorToLight);
        float cosAngleIncidence = max(dot(surfaceNormal, directionToLight), 0.0);
        vec3 lightColor = light.color.xyz * light.color.w * attenuation;

        // Diffuse component
        diffuseLight += lightColor * cosAngleIncidence;

        // Specular component (Blinn-Phong)
        vec3 halfAngle = normalize(directionToLight + viewDirection);
        float blinnTerm = saturate(dot(surfaceNormal, halfAngle));
        blinnTerm = pow(blinnTerm, shininess);

        specularLight += lightColor * blinnTerm * specularIntensity;
    }
}

#endif