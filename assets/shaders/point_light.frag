#version 450

const float PI = 3.14159265359;

layout(constant_id = 0) const int MAX_LIGHTS = 10;

layout(location = 0) in vec2 fragOffset;

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
  vec4 position;
  vec4 color;
  float radius;
} push;

void main() {
    float dis = sqrt(dot(fragOffset, fragOffset));
    if (dis >= 1.0) {
        discard;
    }

    float alpha = 0.5 * (cos(dis * PI) + 1.0);

    outColor = vec4(push.color.xyz, alpha);
}