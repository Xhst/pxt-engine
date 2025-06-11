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
    vec3 light;
    vec3 color;
    vec3 normal;
    float t; // The hit distance (t-value)
    uint seed;
} payload;

// payload used for shadow calculations
layout(location = 1) rayPayloadEXT bool isShadowed;


// Define the ray attributes structure.
// layout(location = 0) hitAttributeEXT is used to receive attributes from the intersection.
// For triangles, this implicitly receives barycentric coordinates.
hitAttributeEXT vec2 HitAttribs;

vec3 sampleCosineWeightedHemisphere(vec2 xi, mat3 TBN)
{
    // Use Malley's method to map uniform random numbers to a cosine-weighted distribution.
    // The direction is initially calculated in a local tangent space where N = (0, 0, 1).
    float phi = 2.0 * PI * xi.x;
    float cosTheta = sqrt(1.0 - xi.y);
    float sinTheta = sqrt(xi.y);
    
    vec3 localDir;
    localDir.x = cos(phi) * sinTheta;
    localDir.y = sin(phi) * sinTheta;
    localDir.z = cosTheta;
    
    // Transform the local direction to world space.
    return normalize(TBN * localDir);
}

vec3 importanceSampleGGX(vec2 xi, mat3 TBN, float roughness) {
    float a = roughness * roughness;
    float phi = 2.0 * PI * xi.x;
    float cosTheta = sqrt((1.0 - xi.y) / (1.0 + (a * a - 1.0 + FLT_EPSILON) * xi.y));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
    vec3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;
    vec3 T, B;

    return normalize(TBN * H);
}

void main()
{
    payload.t = gl_HitTEXT;
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

    vec3 albedo = texture(textures[nonuniformEXT(material.albedoMapIndex)], uv).rgb;
    vec3 emissive = texture(textures[nonuniformEXT(material.emissiveMapIndex)], uv).rgb * material.emissiveColor.rgb * material.emissiveColor.a;
    float metallic = texture(textures[nonuniformEXT(material.metallicMapIndex)], uv).r;
    float perceptualRoughness = texture(textures[nonuniformEXT(material.roughnessMapIndex)], uv).r * 0.9;

    const float roughness = perceptualRoughness * perceptualRoughness;
    uint seed = payload.seed;

    // Base Color
    vec3 finalColor = albedo * instance.textureTintColor.rgb;

    const vec3 F0_dielectric = vec3(0.04);
    vec3 F0 = mix(F0_dielectric, albedo, metallic);

    const vec3 V = -gl_WorldRayDirectionEXT;
    const vec3 N = surfaceNormal;    
        
    vec3 F = brdf_fresnel(F0, max(0.0, dot(N, -V)));
    // Use Russian Roulette to decide if we trace a specular or diffuse ray.
    // The probability is based on the Fresnel reflectance.
    // Metals will have high F, preferring specular paths.
    float specularProbability = max(F.r, max(F.g, F.b));
        
    vec3 newDirection = N;
    // Also factor in metallic property
    specularProbability = mix(specularProbability, 1.0, metallic);
        
    vec2 rand2 = vec2(randomFloat(seed), randomFloat(seed + 1337));

    if (rand2.x < specularProbability) {
        vec3 H = importanceSampleGGX(rand2, TBN, roughness);
        vec3 L = reflect(-V, H); // This is our new direction

        if (dot(N, L) > 0.0) {
            float NoV = max(dot(N, V), FLT_EPSILON);
            float NoL = max(dot(N, L), FLT_EPSILON);
            float NoH = max(dot(N, H), FLT_EPSILON);
            float LoH = max(dot(L, H), FLT_EPSILON);

            float G = brdf_visibility(roughness, NoV, NoL);

            vec3 specular_brdf = F * G * LoH / (NoH * NoV);

            // The Fresnel term F is the color of the specular reflection.
            // We divide by the probability to maintain energy conservation.
            finalColor *= F / specularProbability;
            newDirection = L;
        } else {
            payload.t = -1;
        }
    } else {
        // --- Diffuse Path ---
        if (metallic < 1.0) {
            
            newDirection = sampleCosineWeightedHemisphere(rand2, TBN);
            
            if (dot(N, newDirection) > 0.0) {
                // The diffuse color is scaled by (1.0 - metallic)
                vec3 diffuseColor = albedo * (1.0 - metallic);
                
                // Update throughput: BRDF is color/PI, PDF is NoL/PI. They cancel.
                // We only need to divide by the probability of choosing this path.
                finalColor *= diffuseColor / (1.0 - specularProbability);

                payload.t = gl_HitTEXT; // Mark path as valid
            }
        }
    }
    payload.light = emissive;
    payload.color = finalColor;
    payload.normal = normalize(newDirection);
    
}
