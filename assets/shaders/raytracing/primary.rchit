#version 460

#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_buffer_reference : require
#extension GL_ARB_gpu_shader_int64 : require

#include "../ubo/global_ubo.glsl"
#include "../material/surface_normal.glsl"

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

layout(set = 2, binding = 0) uniform sampler2D textures[];

struct Material {
	vec4 albedoColor;
	int albedoMapIndex;
	int normalMapIndex;
	int ambientOcclusionMapIndex;
	int metallicMapIndex;
	int roughnessMapIndex;
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
    vec4 color; // The color accumulated along the ray
    float t;    // The hit distance (t-value)
} payload;


// Define the ray attributes structure.
// layout(location = 0) hitAttributeEXT is used to receive attributes from the intersection.
// For triangles, this implicitly receives barycentric coordinates.
hitAttributeEXT vec2 HitAttribs;

vec2 barycentricLerp(vec2 a, vec2 b, vec2 c, vec3 barycentrics) {
    return a * barycentrics.x + b * barycentrics.y + c * barycentrics.z;
}

vec3 barycentricLerp(vec3 a, vec3 b, vec3 c, vec3 barycentrics) {
    return a * barycentrics.x + b * barycentrics.y + c * barycentrics.z;
}

vec4 barycentricLerp(vec4 a, vec4 b, vec4 c, vec3 barycentrics) {
    return a * barycentrics.x + b * barycentrics.y + c * barycentrics.z;
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
    vec4 position = barycentricLerp(v0.position, v1.position, v2.position, barycentrics);
    vec4 objectNormal = barycentricLerp(v0.normal, v1.normal, v2.normal, barycentrics);
    vec4 objectTangent = barycentricLerp(v0.tangent, v1.tangent, v2.tangent, barycentrics);
    vec2 uv = barycentricLerp(v0.uv.xy, v1.uv.xy, v2.uv.xy, barycentrics) * instance.textureTilingFactor;

    // Normal Matrix (or Model-View Matrix) used to trasform from object space to world space.
    mat3 normalMatrix = transpose(inverse(mat3(gl_ObjectToWorldEXT)));

    // Calculate the Tangent-Bitangent-Normal (TBN) matrix and the surface normal in world space.
    mat3 TBN = calculateTBN(objectNormal, objectTangent, normalMatrix);
    vec3 surfaceNormal = calculateSurfaceNormal(textures[nonuniformEXT(material.normalMapIndex)], uv, TBN);

    vec4 albedo = texture(textures[nonuniformEXT(material.albedoMapIndex)], uv) * instance.textureTintColor;
    
    payload.color = vec4(surfaceNormal * 0.5 + 0.5, 1.0);
    payload.t = gl_HitTEXT;
}
