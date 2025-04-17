#version 450

layout(constant_id = 0) const int MAX_LIGHTS = 10;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 fragLightPos;

struct PointLight {
  vec4 position;
  vec4 color;
};

layout(set = 0, binding = 0) uniform ShadowUbo {
  mat4 projection;
  // this is a matrix that translates model coordinates to light coordinates
  mat4 lightOriginModel; // we could consider passing this as push constants in the future? (i think no, because we will have too many lights :(  )
  PointLight pointLights[MAX_LIGHTS];
  int numLights;
} ubo;

layout(push_constant) uniform Push {
  // it will be modified to translate the object to the light position (i think so?)
  mat4 modelMatrix;
  // it will be modified to render the different faces
  mat4 cubeFaceView;
} push;


void main() {
  vec4 posWorldFromLight = ubo.lightOriginModel * push.modelMatrix * vec4(position, 1.0);
  gl_Position = ubo.projection * push.cubeFaceView * posWorldFromLight;

  // we pass the positions in object space to be consistent among all (future ;_;) lights.
  fragPos = position;
  fragLightPos = ubo.pointLights[0].position.xyz;
}