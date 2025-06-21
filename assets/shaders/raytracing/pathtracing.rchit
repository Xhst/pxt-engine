#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

#include "../common/math.glsl"
#include "../common/ray.glsl"
#include "../common/payload.glsl"
#include "../common/geometry.glsl"
#include "../common/random.glsl"
#include "../ubo/global_ubo.glsl"
#include "../material/surface_normal.glsl"
#include "../material/pbr/bsdf.glsl"

// Min depth for Russian Roulette termination
#define MIN_DEPTH 3

struct Material {
	vec4 albedoColor;
    vec4 emissiveColor;
	int albedoMapIndex;
	int normalMapIndex;
	int ambientOcclusionMapIndex;
	int metallicMapIndex;
	int roughnessMapIndex;
    int emissiveMapIndex;
};

struct MeshInstanceDescription {
    uint64_t vertexAddress;  
    uint64_t indexAddress;   
    uint materialIndex; 
    float textureTilingFactor;
    vec4 textureTintColor;
};

struct Emitter {
    uint instanceIndex;
    uint numberOfFaces;
};

layout(set = 1, binding = 0) uniform accelerationStructureEXT TLAS;

layout(set = 2, binding = 0) uniform sampler2D textures[];

layout(set = 4, binding = 0) readonly buffer materialsSSBO {
    Material m[];
} materials;

layout(set = 5, binding = 0) uniform samplerCube skyboxSampler;

layout(set = 6, binding = 0, std430) readonly buffer meshInstancesSSBO {
    MeshInstanceDescription i[]; 
} meshInstances;

// --- Payloads ---
layout(location = 0) rayPayloadInEXT PathTracePayload p_pathTrace;
layout(location = 1) rayPayloadEXT bool p_isVisible;

// For triangles, this implicitly receives barycentric coordinates.
hitAttributeEXT vec2 barycentrics;

vec3 getAlbedo(const Material material, const vec2 uv, const vec4 tintColor) {
    vec3 albedo = texture(textures[material.albedoMapIndex], uv).rgb;
    return albedo * tintColor.rgb;
}

vec3 getNormal(const Material material, const vec2 uv) {
    return texture(textures[material.normalMapIndex], uv).rgb;
}

vec3 getEmission(const Material material, const vec2 uv) {
    vec3 emissive = texture(textures[material.emissiveMapIndex], uv).rgb;

    // The alpha channels is used as intensity
    return emissive * material.emissiveColor.rgb * material.emissiveColor.a;
}

float getRoughness(const Material material, const vec2 uv) {
    return texture(textures[material.roughnessMapIndex], uv).r;
}

float getMetalness(const Material material, const vec2 uv) {
    return texture(textures[material.metallicMapIndex], uv).r;
}

void indirectLighting(SurfaceData surface, vec3 outLightDir, out vec3 inLightDir) {
    float pdf;

    vec3 brdf_multiplier = sampleBSDF(surface, outLightDir, inLightDir, pdf, p_pathTrace.seed);

    if (brdf_multiplier == vec3(0.0)) {
        // No contribution from this surface
        p_pathTrace.done = true;
        return;
    }

    p_pathTrace.throughput *= brdf_multiplier;

    // Apply Russian Roulette termination 
    if (p_pathTrace.depth > MIN_DEPTH) {
        // Calculate the Russian Roulette probability based on the max component of the throughput
        // to ensure that the path is terminated with a probability proportional to its contribution.
        float russianRouletteProbability = maxComponent(p_pathTrace.throughput);

        if (randomFloat(p_pathTrace.seed) > russianRouletteProbability) {
            p_pathTrace.done = true;
            return;
        }

        // Scale the throughput by the inverse of the Russian Roulette probability
        // to ensure energy conservation.
        p_pathTrace.throughput /= russianRouletteProbability;
    }
}

void main() {
    MeshInstanceDescription instance = meshInstances.i[gl_InstanceCustomIndexEXT];
    Material material = materials.m[instance.materialIndex];
    Triangle triangle = getTriangle(instance.indexAddress, instance.vertexAddress, gl_PrimitiveID);

    const vec2 uv = getTextureCoords(triangle, barycentrics) * instance.textureTilingFactor;

    // Tangent, Bi-tangent, Normal (TBN) matrix to transform tangent space to world space
    mat3 tbn = calculateTBN(triangle, mat3(gl_ObjectToWorld3x4EXT), barycentrics);
    const vec3 surfaceNormal = calculateSurfaceNormal(textures[nonuniformEXT(material.normalMapIndex)], uv, tbn);
    
    vec3 worldNormal = tbn[2];

    SurfaceData surface;
    surface.normal = worldToTangent(tbn, surfaceNormal);
    surface.albedo = getAlbedo(material, uv, instance.textureTintColor);
    surface.metalness = getMetalness(material, uv);
    surface.roughness = getRoughness(material, uv);
    surface.reflectance = calculateReflectance(surface.albedo, surface.metalness);
    surface.specularProbability = calculateSpecularProbability(surface.albedo, surface.metalness, surface.reflectance);

    vec3 emission = getEmission(material, uv);
    p_pathTrace.radiance += emission;

    if (maxComponent(emission) > 0.0) {
        p_pathTrace.done = true;
        return;
    }

    const vec3 worldPosition = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_RayTmaxEXT;

    // The ray has the opposite direction to that of the light.
    // The "incoming" ray hitting the surface is actually the outgoing light direction.
    // We perform calculation in tangent space
    vec3 outgoingLightDirection = worldToTangent(tbn, -gl_WorldRayDirectionEXT);
    vec3 incomingLightDirection;

    indirectLighting(surface, outgoingLightDirection, incomingLightDirection);

    // Convert back to world space
    outgoingLightDirection = tangentToWorld(tbn, incomingLightDirection);

    // Update the payload for the next bounce
    p_pathTrace.depth++;
    p_pathTrace.origin = worldPosition + worldNormal * FLT_EPSILON;
    p_pathTrace.direction = outgoingLightDirection;
}