#version 460

#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

#include "../common/math.glsl"
#include "../common/ray.glsl"
#include "../common/random.glsl"
#include "../ubo/global_ubo.glsl"
#include "../material/surface_normal.glsl"
#include "../lighting/blinn_phong_lighting.glsl"
#include "../material/pbr/brdf.glsl"

struct Vertex {
    vec4 position;  // Position of the vertex.
    vec4 normal;    // Normal vector for lighting calculations.
    vec4 tangent;   // Tangent vector for lighting calculations.
    vec4 uv;        // Texture coordinates for the vertex.
};

/**
 * References of the vertex buffers.
 * It can be used to access vertex data using the buffer address (uint64_t)
 */
layout(buffer_reference, buffer_reference_align = 16, std430) readonly buffer VertexBuffer {
    Vertex v[];
};

/**
 * References of the index buffers.
 * It can be used to access index data using the buffer address (uint64_t)
 * The indices are stored as uint32 values, and each triangle is represented by 3 indices.
 */
layout(buffer_reference, buffer_reference_align = 16, std430) readonly buffer IndexBuffer {
    uint i[]; 
};

layout(set = 1, binding = 0) uniform accelerationStructureEXT TLAS; // Used for shadows

layout(set = 2, binding = 0) uniform sampler2D textures[];

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

layout(set = 4, binding = 0) readonly buffer materials {
    Material materials[];
} materialsSSBO;

struct MeshInstanceDescription {
    uint64_t vertexAddress;  
    uint64_t indexAddress;   
    uint materialIndex; 
    float textureTilingFactor;
    vec4 textureTintColor;
};

layout(set = 6, binding = 0, std430) readonly buffer meshInstances {
    MeshInstanceDescription instances[]; 
} meshInstancesSSBO;


// Define the ray payload structure. Must match the raygen and miss shaders.
layout(location = 0) rayPayloadInEXT struct RayPayload {
    // Accumulated color and energy along the path.
    vec3 radiance;

    // The accumulated material reflectance/transmittance. It's multiplied at each bounce.
    vec3 throughput;

    // Current number of bounces. Used to limit path length and for Russian Roulette.
    int depth;

    // The origin of the ray in world space.
    vec3 origin; 

    // The direction of the ray in world space.
    vec3 direction;

    // A flag to signal that the path has been terminated (e.g., hit the sky, absorbed).
    bool done;

    // A seed for the random number generator, updated at each bounce.
    uint seed;
} payload;

// Define the ray attributes structure.
// layout(location = 0) hitAttributeEXT is used to receive attributes from the intersection.
// For triangles, this implicitly receives barycentric coordinates.
hitAttributeEXT vec2 HitAttribs;


float D_GGX(float NdotH, float roughness) {
    float a2 = roughness * roughness;
    float d = NdotH * NdotH * (a2 - 1.0) + 1.0;
    return a2 / (PI * d * d);
}

// Geometry Function (Smith's method with Schlick-GGX)
float G_Schlick_GGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0; // k for direct lighting
    return NdotV / (NdotV * (1.0 - k) + k);
}

float G_Smith(float NdotV, float NdotL, float roughness) {
    return G_Schlick_GGX(NdotV, roughness) * G_Schlick_GGX(NdotL, roughness);
}

// Fresnel Function (Schlick's approximation)
vec3 F_Schlick(float HdotV, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - HdotV, 0.0, 1.0), 5.0);
}

// Generates a sample direction on a cosine-weighted hemisphere using the provided TBN matrix.
vec3 sampleCosineWeightedHemisphere(inout uint seed, const mat3 TBN) {
    float r1 = randomFloat(seed);
    float r2 = randomFloat(seed);
    float phi = 2.0 * PI * r1;
    float cosTheta = sqrt(1.0 - r2);
    float sinTheta = sqrt(r2);
    vec3 sampleDir_local = vec3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
    // Transform from local tangent space to world space using the TBN matrix.
    return TBN * sampleDir_local;
}

// Generates an importance-sampled microfacet normal (H) for GGX using the TBN matrix.
vec3 importanceSampleGGX(inout uint seed, const mat3 TBN, float roughness) {
    float r1 = randomFloat(seed);
    float r2 = randomFloat(seed);
    float a = roughness * roughness;
    float phi = 2.0 * PI * r1;
    float cosTheta = sqrt((1.0 - r2) / (1.0 + (a * a - 1.0 + FLT_EPSILON) * r2));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
    vec3 H_local = vec3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
    // Transform from local tangent space to world space.
    return normalize(TBN * H_local);
}


// The main sampleBSDF function now takes the TBN matrix instead of just the normal.
vec3 sampleBSDF(vec3 V, vec3 surfaceNormal, mat3 TBN, vec3 albedo, float metallic, float roughness, inout uint seed, out vec3 L) {
    vec3 N = surfaceNormal;
    vec3 Wo = -V;

    const vec3 F0_dielectric = vec3(0.04);
    vec3 F0 = mix(F0_dielectric, albedo, metallic);

    float specularProbability = max(F0.r, max(F0.g, F0.b));
    specularProbability = mix(specularProbability, 1.0, metallic);

    if (randomFloat(seed) < specularProbability) {
        // --- Specular Lobe ---
        vec3 H = importanceSampleGGX(seed, TBN, roughness);
        L = reflect(Wo, H);

        if (dot(N, L) <= 0.0) return vec3(0.0);

        float NdotL = max(abs(dot(N, L)), FLT_EPSILON);
        float NdotWo = max(abs(dot(N, Wo)), FLT_EPSILON);
        float NdotH = max(abs(dot(N, H)), FLT_EPSILON);
        float HdotWo = max(abs(dot(H, Wo)), FLT_EPSILON);

        vec3 F = brdf_fresnel(F0, HdotWo);
        float G = G_Smith(NdotWo, NdotL, roughness);

        float denominator = NdotWo * NdotH;
        vec3 specular_brdf = F * G * HdotWo / denominator;

        return specular_brdf / specularProbability;
    } else {
        // --- Diffuse Lobe ---
        L = sampleCosineWeightedHemisphere(seed, TBN);

        if (dot(N, L) <= 0.0) return vec3(0.0);

        return albedo * (1.0 - metallic) / (1.0 - specularProbability);
    }
}


void main()
{
    MeshInstanceDescription instance = meshInstancesSSBO.instances[gl_InstanceCustomIndexEXT];

    IndexBuffer indices = IndexBuffer(instance.indexAddress);
    VertexBuffer vertices = VertexBuffer(instance.vertexAddress);
    Material material = materialsSSBO.materials[instance.materialIndex];

    // Retrieve the indices of the triangle being hit.
    uint i0 = indices.i[gl_PrimitiveID * 3 + 0];
    uint i1 = indices.i[gl_PrimitiveID * 3 + 1];
    uint i2 = indices.i[gl_PrimitiveID * 3 + 2];

    // Retrieve the vertices of the triangle using the indices.
    Vertex v0 = vertices.v[i0];
    Vertex v1 = vertices.v[i1];
    Vertex v2 = vertices.v[i2];

    // Calculate barycentric coordinates from the hit attributes.
    const vec3 barycentrics = vec3(1.0 - HitAttribs.x - HitAttribs.y, HitAttribs.x, HitAttribs.y);
    
    // Interpolate the vertex attributes using barycentric coordinates.
    const vec4 position = barycentricLerp(v0.position, v1.position, v2.position, barycentrics);
    const vec4 objectNormal = barycentricLerp(v0.normal, v1.normal, v2.normal, barycentrics);
    const vec4 objectTangent = barycentricLerp(v0.tangent, v1.tangent, v2.tangent, barycentrics);
    const vec2 uv = barycentricLerp(v0.uv.xy, v1.uv.xy, v2.uv.xy, barycentrics) * instance.textureTilingFactor;

    // Normal Matrix (or Model-View Matrix) used to trasform from object space to world space.
    // its just the inverse of the gl_ObjectToWorld3x4EXT
    const mat3 normalMatrix = mat3(gl_WorldToObject3x4EXT);
    vec3 worldNormal = normalize(normalMatrix * objectNormal.xyz);

    // Calculate the Tangent-Bitangent-Normal (TBN) matrix and the surface normal in world space.
    const mat3 TBN = calculateTBN(objectNormal, objectTangent, normalMatrix);
    const vec3 surfaceNormal = calculateSurfaceNormal(textures[nonuniformEXT(material.normalMapIndex)], uv, TBN);


    // calculate world position
    const vec3 worldPosition = vec3(gl_ObjectToWorldEXT * position);

    vec3 albedo = texture(textures[nonuniformEXT(material.albedoMapIndex)], uv).rgb * instance.textureTintColor.rgb;
    vec3 emissive = texture(textures[nonuniformEXT(material.emissiveMapIndex)], uv).rgb * material.emissiveColor.rgb * material.emissiveColor.a;
    float metallic = texture(textures[nonuniformEXT(material.metallicMapIndex)], uv).r;
    float perceptualRoughness = texture(textures[nonuniformEXT(material.roughnessMapIndex)], uv).r;

    const float roughness = perceptualRoughness * perceptualRoughness;

    const vec3 V = -gl_WorldRayDirectionEXT;
    const vec3 N = surfaceNormal;    
    
    payload.depth++;

    // If the surface emits light, we add its contribution to the path's radiance and terminate.
    if (dot(emissive, emissive) > 0.0) {
        payload.radiance += emissive * payload.throughput;
        payload.done = true;
        return; 
    }

    float survivalProb = 1.0;
    if (payload.depth > 3) {
        survivalProb = max(payload.throughput.r, max(payload.throughput.g, payload.throughput.b));
        if (randomFloat(payload.seed) > survivalProb) {
            payload.done = true;
            return; // Path is terminated/absorbed
        }
        // If the path survives, we must scale its throughput to compensate for the absorbed energy.
        payload.throughput /= survivalProb;
    }

    vec3 L; // New direction
    vec3 bsdfContribution = sampleBSDF(V, surfaceNormal, TBN, albedo, metallic, roughness, payload.seed, L);

    payload.throughput *= bsdfContribution;

    // If throughput is close to zero, no more light can be contributed, so we stop.
    if (dot(payload.throughput, payload.throughput) < 0.0001) {
        payload.done = true;
        return;
    }

    payload.origin = worldPosition + worldNormal * 0.0001;
    payload.direction = L;
    
}
