#version 460

#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

#include "../common/math.glsl"
#include "../common/ray.glsl"
#include "../common/geometry.glsl"
#include "../ubo/global_ubo.glsl"
#include "../material/surface_normal.glsl"
#include "../lighting/blinn_phong_lighting.glsl"

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

layout(set = 2, binding = 0) uniform sampler2D textures[];

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

// payload used for shadow calculations
layout(location = 1) rayPayloadEXT bool isShadowed;


// Define the ray attributes structure.
// layout(location = 0) hitAttributeEXT is used to receive attributes from the intersection.
// For triangles, this implicitly receives barycentric coordinates.
hitAttributeEXT vec2 HitAttribs;

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
    // Interpolate the vertex attributes using barycentric coordinates.
    const vec4 position = barycentricLerp(v0.position, v1.position, v2.position, HitAttribs);
    const vec4 objectNormal = barycentricLerp(v0.normal, v1.normal, v2.normal, HitAttribs);
    const vec4 objectTangent = barycentricLerp(v0.tangent, v1.tangent, v2.tangent, HitAttribs);
    const vec2 uv = barycentricLerp(v0.uv.xy, v1.uv.xy, v2.uv.xy, HitAttribs) * instance.textureTilingFactor;

    // Normal Matrix (or Model-View Matrix) used to trasform from object space to world space.
    // its just the inverse of the gl_ObjectToWorld3x4EXT
    const mat3 normalMatrix = mat3(gl_WorldToObject3x4EXT);
    vec3 worldNormal = normalize(normalMatrix * objectNormal.xyz);

    // Calculate the Tangent-Bitangent-Normal (TBN) matrix and the surface normal in world space.
    const mat3 TBN = calculateTBN(objectNormal, objectTangent, normalMatrix);
    const vec3 surfaceNormal = calculateSurfaceNormal(textures[nonuniformEXT(material.normalMapIndex)], uv, TBN);

    // calculate world position
    const vec3 worldPosition = vec3(gl_ObjectToWorldEXT * position);

    // compute diffuse and specular
    vec3 specularLight, diffuseLight;
    const vec3 viewDirection = gl_WorldRayDirectionEXT;
    computeBlinnPhongLighting(surfaceNormal, viewDirection, worldPosition, 1.0, 0.0, diffuseLight, specularLight);

    vec4 albedo = texture(textures[nonuniformEXT(material.albedoMapIndex)], uv);

    // shadow
    float attenuation = 1.0;

    // Tracing shadow ray only if the light is visible from the surface
    vec3 lightPosition = ubo.pointLights[0].position.xyz;
    vec3 vecToLight = lightPosition - worldPosition;
    float lightDistance = length(vecToLight);
    vec3 dirToLight = normalize(vecToLight);

    // We assume by default that everything is in shadow
    // then we cast a ray and set to false if the ray misses
    isShadowed = true; 

    if(dot(worldNormal, dirToLight) > 0) {
        // A small bias to avoid the shadow terminator problem
        const float bias = 0.007;

        Ray shadowRay;
        shadowRay.origin = worldPosition + worldNormal * bias;
        shadowRay.direction = dirToLight;
        float tMin   = 0.001;
        float tMax   = lightDistance;
        uint  flags  = gl_RayFlagsTerminateOnFirstHitEXT | gl_RayFlagsOpaqueEXT | gl_RayFlagsSkipClosestHitShaderEXT;
        
        traceRayEXT(TLAS,        // acceleration structure
                    flags,       // rayFlags
                    0xFF,        // cullMask
                    0,           // sbtRecordOffset
                    0,           // sbtRecordStride
                    1,           // missIndex
                    shadowRay.origin,      
                    tMin,        // ray min range
                    shadowRay.direction,      
                    tMax,        // ray max range
                    1            // payload (location = 1)
        );
    }

    if(isShadowed) {
        attenuation = 0.4;
    }

    // Base Color
    vec3 finalColor = albedo.rgb * instance.textureTintColor.rgb;

    // Apply lighting
    finalColor = (diffuseLight + specularLight) * finalColor * attenuation;
    
    payload.color = vec4(finalColor, 1.0);
    payload.t = gl_HitTEXT;
}
