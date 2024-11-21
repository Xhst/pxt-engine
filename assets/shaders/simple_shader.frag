#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragPosWorld;
layout(location = 2) in vec3 fragNormalWorld;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform GlobalUbo {
  mat4 projectionViewMatrix;
  vec4 ambientLightColor;
  vec3 lightPosition;
  vec4 lightColor; //4th component is intensity
} ubo;

layout(push_constant) uniform Push {
  mat4 modelMatrix;
  mat4 normalMatrix;
} push;

void main() {
  vec3 vectorToLight = ubo.lightPosition - fragPosWorld;
  float attenuation = 1.0 / dot(vectorToLight, vectorToLight);

  vec3 lightColor = ubo.lightColor.xyz * ubo.lightColor.w * attenuation; // color * intensity
  vec3 ambientLightColor = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w; // color * intensity
  vec3 diffuseLight = lightColor * max(dot(normalize(fragNormalWorld), normalize(vectorToLight)), 0); // fragment normal could not be normalized from previous stage (normals interpolation)

  // r g b a
  outColor = vec4((diffuseLight + ambientLightColor) * fragColor, 1.0);
}