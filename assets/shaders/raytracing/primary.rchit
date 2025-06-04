#version 460

#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_buffer_reference : require
#extension GL_ARB_gpu_shader_int64 : require

// Include the global UBO definition
#include "../ubo/global_ubo.glsl"

struct Vertex {
    vec4 position;  // Position of the vertex.
    vec4 normal;    // Normal vector for lighting calculations.
    vec4 tangent;   // Tangent vector for lighting calculations.
    vec4 uv;        // Texture coordinates for the vertex.
};

layout(buffer_reference, buffer_reference_align = 16, std430) readonly buffer VertexBuffer {
    Vertex v[];
};

layout(buffer_reference, buffer_reference_align = 16, scalar) readonly buffer IndexBuffer {
    uint i[]; 
};

// Declare the textures binding.
// The descriptor_indexing extension allows using arrays of textures with a runtime-sized descriptor array.
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
    MeshInstanceDescription instances[]; // Array of mesh instances.
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

vec2 BaryLerp(vec2 a, vec2 b, vec2 c, vec3 barycentrics) {
    return a * barycentrics.x + b * barycentrics.y + c * barycentrics.z;
}

vec3 BaryLerp(vec3 a, vec3 b, vec3 c, vec3 barycentrics) {
    return a * barycentrics.x + b * barycentrics.y + c * barycentrics.z;
}

void main()
{
    // This shader is executed when a ray hits the closest geometry.

    // Store the hit distance in the payload
    payload.t = gl_HitTEXT;

    MeshInstanceDescription instance = meshInstancesSSBO.instances[gl_InstanceCustomIndexEXT];
    Material material = materialsSSBO.materials[instance.materialIndex];

    IndexBuffer indices = IndexBuffer(instance.indexAddress);
    VertexBuffer vertices = VertexBuffer(instance.vertexAddress);

    uint i0 = indices.i[gl_PrimitiveID * 3 + 0];
    uint i1 = indices.i[gl_PrimitiveID * 3 + 1];
    uint i2 = indices.i[gl_PrimitiveID * 3 + 2];

    Vertex v0 = vertices.v[i0];
    Vertex v1 = vertices.v[i1];
    Vertex v2 = vertices.v[i2];

    const vec3 barycentrics = vec3(1.0 - HitAttribs.x - HitAttribs.y, HitAttribs.x, HitAttribs.y);
    //const vec3 normal = normalize(BaryLerp(v0.normal.xyz, v1.normal.xyz, v2.normal.xyz, barycentrics));
    vec2 uv = BaryLerp(v0.uv.xy, v1.uv.xy, v2.uv.xy, barycentrics) * instance.textureTilingFactor;

    vec4 albedo = texture(textures[nonuniformEXT(material.albedoMapIndex)], uv) * instance.textureTintColor;
    
    payload.color = vec4(albedo.rgb, 1.0);
}
