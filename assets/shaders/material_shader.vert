#version 450

layout(constant_id = 0) const int MAX_LIGHTS = 10;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec4 tangent;
layout(location = 4) in vec2 uv;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragPosWorld;
layout(location = 2) out vec3 fragNormalWorld;
layout(location = 3) out vec2 fragUV;
layout(location = 4) out mat3 fragTBN;

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
	vec4 color;
	float specularIntensity;
	float shininess;
	int textureIndex;
	int normalMapIndex;
	int ambientOcclusionMapIndex;
	float tilingFactor;
} push;


void main() {
	vec4 positionWorld = push.modelMatrix * vec4(position, 1.0);
	gl_Position = ubo.projectionMatrix * ubo.viewMatrix * positionWorld;

	mat3 normalMatrix3 = mat3(push.normalMatrix);

	vec3 norm = normalize(normalMatrix3 * normal);

	// Gram–Schmidt processA
	vec3 tang = (tangent.xyz - dot(normal, tangent.xyz) * normal);

	// tangent.w is the handedness
	vec3 bitang = cross(norm, tang) * tangent.w;
 
	fragColor = vec3(push.color);
	fragPosWorld = positionWorld.xyz;
	fragNormalWorld = norm;
	fragUV = uv;
	fragTBN = mat3(tang, bitang, norm);
}