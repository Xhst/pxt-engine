#version 450
#extension GL_GOOGLE_include_directive : require

#include "ubo/shadow_ubo.glsl"

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec4 tangent;
layout(location = 4) in vec2 uv;

layout(location = 0) out vec3 fragPosWorld;
layout(location = 1) out vec3 fragLightPos;

layout(push_constant) uniform Push {
  // it will be modified to translate the object to the light position (i think so?)
  mat4 modelMatrix;
  // it will be modified to render the different faces
  mat4 cubeFaceView;
} push;


void main() {
  vec4 posWorld = push.modelMatrix * vec4(position, 1.0);
  vec4 posWorldFromLight = ubo.lightOriginModel * posWorld;
  gl_Position = ubo.projection * push.cubeFaceView * posWorldFromLight;

  fragPosWorld = posWorld.xyz;
  fragLightPos = ubo.pointLights[0].position.xyz;
}