#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout (location = 0) out vec3 fragColor;

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
  vec4 positionWorld = push.modelMatrix * vec4(position, 1.0);
  gl_Position = ubo.projectionViewMatrix * positionWorld;
 
  vec3 normalWorldSpace = normalize(mat3(push.normalMatrix) * normal);

  vec3 vectorToLight = ubo.lightPosition - positionWorld.xyz;
  float attenuation = 1.0 / dot(vectorToLight, vectorToLight);

  vec3 lightColor = ubo.lightColor.xyz * ubo.lightColor.w * attenuation; // color * intensity
  vec3 ambientLightColor = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w; // color * intensity
  vec3 diffuseLight = lightColor * max(dot(normalWorldSpace, normalize(vectorToLight)), 0);

  fragColor = (diffuseLight + ambientLightColor) * color;
}