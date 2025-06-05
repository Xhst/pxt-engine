#version 460
#extension GL_GOOGLE_include_directive : require

#include "ubo/global_ubo.glsl"
#include "material/surface_normal.glsl"

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 normal;
layout(location = 2) in vec4 tangent;
layout(location = 3) in vec4 uv;

layout(location = 0) out vec3 fragPosWorld;
layout(location = 1) out vec3 fragNormalWorld;
layout(location = 2) out vec2 fragUV;
layout(location = 3) out mat3 fragTBN;

layout(push_constant) uniform Push {
	mat4 modelMatrix;
	mat4 normalMatrix;
	vec4 color;
	int enableWireframe;
	int enableNormalsColor;
	int textureIndex;
	int normalMapIndex;
	int ambientOcclusionMapIndex;
	float tilingFactor;
} push;


void main() {
	vec4 positionWorld = push.modelMatrix * position;
	gl_Position = ubo.projectionMatrix * ubo.viewMatrix * positionWorld;
 
	mat3 TBN = calculateTBN(normal, tangent, mat3(push.normalMatrix));

	vec3 worldNormal = normalize(vec3(push.normalMatrix * normal));

	fragPosWorld = positionWorld.xyz;
	fragNormalWorld = worldNormal;
	fragUV = uv.xy;
	fragTBN = TBN;
}