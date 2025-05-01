#version 450
#extension GL_EXT_nonuniform_qualifier : enable

layout(constant_id = 0) const int MAX_LIGHTS = 10;

layout(location = 0) in vec3 fragPosWorld;
layout(location = 1) in vec3 fragNormalWorld;
layout(location = 2) in vec2 fragUV;
layout(location = 3) in mat3 fragTBN;

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

layout(push_constant) uniform Push {
	mat4 modelMatrix;
	mat4 normalMatrix;
    int normalMapIndex;
	int enableWireframe;
	int enableNormals;
} push;

/*
 * Extract and transform surface normal from the normal map.
 *
 * Samples the normal from a normal map, converts it from [0,1] to [-1,1],
 * and transforms it into world space using the TBN matrix.
 */
vec3 getSurfaceNormal(vec2 texCoords) {
    //vec3 normalMapValue = texture(textures[push.normalMapIndex], texCoords).rgb;
    //normalMapValue = normalMapValue * 2.0 - 1.0;
    //return normalize(fragTBN * normalMapValue);

    return normalize(fragNormalWorld);
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
        blinnTerm = pow(blinnTerm, 0);

        specularLight += lightColor * blinnTerm * 1;
    }
}

void main() {
    vec3 fragColor = vec3(1.0, 1.0, 1.0); // Default color

    if (push.enableWireframe == 1) {
        outColor = vec4(0.0, 1.0, 0.0, 1.0);
        return;
    }

    vec2 texCoords = fragUV; //* push.tilingFactor;

    vec3 surfaceNormal = getSurfaceNormal(texCoords);

    if (push.enableNormals == 1) {
        outColor = vec4(surfaceNormal * 0.5 + 0.5, 1.0);
        return;
    }

    vec3 cameraPosWorld = ubo.inverseViewMatrix[3].xyz;
    vec3 viewDirection = normalize(cameraPosWorld - fragPosWorld);

    vec3 diffuseLight, specularLight;
    computeLighting(surfaceNormal, viewDirection, diffuseLight, specularLight);

    // we need to add control coefficients to regulate both terms (diffuse/specular)
    // for now we use fragColor for both which is ideal for metallic objects
    vec3 baseColor = (diffuseLight * fragColor + specularLight * fragColor);

    outColor = vec4(baseColor, 1.0);
}
