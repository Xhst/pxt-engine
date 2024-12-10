#version 450

layout(constant_id = 0) const int MAX_LIGHTS = 10;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragPosWorld;
layout(location = 2) in vec3 fragNormalWorld;

layout(location = 0) out vec4 outColor;

struct PointLight {
  vec4 position;
  vec4 color;
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
} push;

void main() {
  vec3 diffuseLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
  vec3 specularLight = vec3(0.0);
  vec3 surfaceNormal = normalize(fragNormalWorld); // fragment normal could not be normalized from previous stage (normals interpolation)

  vec3 cameraPosWorld = ubo.inverseViewMatrix[3].xyz;
  vec3 viewDirection = normalize(cameraPosWorld - fragPosWorld); // it's the direction from fragment to camera

  for (int i = 0; i < ubo.numLights; i++) {
    PointLight light = ubo.pointLights[i];
    vec3 vectorToLight = light.position.xyz - fragPosWorld;
    float attenuation = 1.0 / dot(vectorToLight, vectorToLight);
    vec3 directionToLight = normalize(vectorToLight);

    float cosAngleIncidence = max(dot(surfaceNormal, directionToLight), 0);
    vec3 lightColor = light.color.xyz * light.color.w * attenuation; // color * intensity * attenuation
    
    diffuseLight += lightColor * cosAngleIncidence;

    // specualar light - using Blinn-Phong model
    vec3 halfAngle = normalize(directionToLight + viewDirection);
    float blinnTerm = dot(surfaceNormal, halfAngle);
    blinnTerm = clamp(blinnTerm, 0 , 1);
    blinnTerm = pow(blinnTerm, 32.0); // 32 can be passed based on material. Higher values -> sharper highlight

    specularLight += lightColor * blinnTerm; 
  }

  /* we need to add control coefficients to regulate both terms
     for now we use fragColor for both which is ideal for metallic objects
  */
  outColor = vec4(diffuseLight * fragColor + specularLight * fragColor, 1.0);
}