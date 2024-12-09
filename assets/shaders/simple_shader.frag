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
  vec3 surfaceNormal = normalize(fragNormalWorld); // fragment normal could not be normalized from previous stage (normals interpolation)

  for (int i = 0; i < ubo.numLights; i++) {
    PointLight light = ubo.pointLights[i];
    vec3 vectorToLight = light.position.xyz - fragPosWorld;
    float attenuation = 1.0 / dot(vectorToLight, vectorToLight);
    float cosAngleIncidence = max(dot(surfaceNormal, normalize(vectorToLight)), 0);
    vec3 lightColor = light.color.xyz * light.color.w * attenuation; // color * intensity * attenuation
    
    diffuseLight += lightColor * cosAngleIncidence;
  }

  // r g b a
  outColor = vec4(diffuseLight * fragColor, 1.0);
}