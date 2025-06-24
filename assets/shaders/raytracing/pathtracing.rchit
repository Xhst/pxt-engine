#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_debug_printf : enable
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
#include "sky.glsl"

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
    mat4 objectToWorld;
    mat4 worldToObject;
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

layout(set = 6, binding = 0, std430) readonly buffer meshInstancesSSBO {
    MeshInstanceDescription i[]; 
} meshInstances;

layout(set = 7, binding = 0, std430) readonly buffer emittersSSBO {
    uint numEmitters;
    Emitter e[]; 
} emitters;

// --- Payloads ---
layout(location = PathTracePayloadLocation) rayPayloadInEXT PathTracePayload p_pathTrace;
layout(location = VisibilityPayloadLocation) rayPayloadEXT bool p_isVisible;

// For triangles, this implicitly receives barycentric coordinates.
hitAttributeEXT vec2 barycentrics;

vec3 getAlbedo(const Material material, const vec2 uv, const vec4 tintColor) {
    vec3 albedo = texture(textures[nonuniformEXT(material.albedoMapIndex)], uv).rgb;
    return albedo * tintColor.rgb;
}

vec3 getNormal(const Material material, const vec2 uv) {
    return texture(textures[nonuniformEXT(material.normalMapIndex)], uv).rgb;
}

vec3 getEmission(const Material material, const vec2 uv) {
    vec3 emissive = texture(textures[nonuniformEXT(material.emissiveMapIndex)], uv).rgb;

    // The alpha channels is used as intensity
    return emissive * material.emissiveColor.rgb * material.emissiveColor.a;
}

float getRoughness(const Material material, const vec2 uv) {
    return pow2(texture(textures[nonuniformEXT(material.roughnessMapIndex)], uv).r);
}

float getMetalness(const Material material, const vec2 uv) {
    return texture(textures[nonuniformEXT(material.metallicMapIndex)], uv).r;
}

struct EmitterSample {
    vec3 radiance;
    vec3 inLightDir;
    float lightDistance;
    float pdf;
    bool isVisible; 
};

void sampleEmitter(
    SurfaceData surface,
    vec3 worldPosition, 
    out EmitterSample smpl
) {
    smpl.radiance = vec3(0.0);
    smpl.lightDistance = FLT_MAX;
    smpl.pdf = 0.0;
    smpl.isVisible = false;

    const uint numEmitters = uint(emitters.numEmitters);

    if (numEmitters == 0) {
        return;
    }
    
    // We add one extra emitters for the sky
    const uint totalSamplableEmitters = numEmitters + 1;

    const uint emitterIndex = nextUint(p_pathTrace.seed, totalSamplableEmitters);

    vec3 worldInLightDir = vec3(0.0);

    if (emitterIndex == numEmitters) {
        // Sample the sky as an emitter
        smpl.inLightDir = sampleCosineWeightedHemisphere(randomVec2(p_pathTrace.seed));

        worldInLightDir = tangentToWorld(surface.tbn, smpl.inLightDir);

        smpl.pdf = pdfCosineWeightedHemisphere(max(smpl.inLightDir.z, 0)) / totalSamplableEmitters;
        smpl.radiance = getSkyRadiance(worldInLightDir);

        if (smpl.radiance == vec3(0.0)) return;

    } else {
        // Sample a mesh emitter
        const Emitter emitter = emitters.e[emitterIndex];
        MeshInstanceDescription emitterInstance = meshInstances.i[emitter.instanceIndex];
        Material material = materials.m[emitterInstance.materialIndex];
        
        uint faceIndex = nextUint(p_pathTrace.seed, emitter.numberOfFaces);

        // Generate barycentric coordinates for the triangle
        vec2 u = randomVec2(p_pathTrace.seed);
        float uxsqrt = sqrt(u.x);
        vec2 emitterBarycentrics = vec2(1.0 - uxsqrt, u.y * uxsqrt);
    
        Triangle emitterTriangle = getTriangle(emitterInstance.indexAddress, emitterInstance.vertexAddress, faceIndex);

        vec2 uv = getTextureCoords(emitterTriangle, emitterBarycentrics) * emitterInstance.textureTilingFactor;
        smpl.radiance = getEmission(material, uv);

        if (smpl.radiance == vec3(0.0)) {
            return; 
        }

        vec3 emitterObjPosition = getPosition(emitterTriangle, emitterBarycentrics);
        vec3 emitterObjNormal = getNormal(emitterTriangle, emitterBarycentrics);

        mat4 emitterObjectToWorld = mat4(emitterInstance.objectToWorld);
        // The upper 3x3 of the world-to-object matrix is the normal matrix
        mat3 emitterNormalMatrix = mat3(emitterInstance.worldToObject);

        vec3 emitterPosition = vec3(emitterObjectToWorld * vec4(emitterObjPosition, 1.0));
        vec3 emitterNormal = normalize(emitterNormalMatrix * emitterObjNormal);

        // vector from emitter the surface to the emitter
        vec3 outLightVec = worldPosition - emitterPosition;

        smpl.lightDistance = length(outLightVec);

        float areaWorld = calculateWorldSpaceTriangleArea(emitterTriangle, mat3(emitterInstance.objectToWorld));

        if (areaWorld <= 0.0 || smpl.lightDistance <= 0) {
            return;
        }

        vec3 outLightDir = outLightVec / smpl.lightDistance;

        float emitterCosTheta = cosTheta(emitterNormal, outLightDir);
        if (emitterCosTheta <= 0.0) {
            return;
        }

        worldInLightDir = -outLightDir; 
        smpl.inLightDir = worldToTangent(surface.tbn, worldInLightDir);
        smpl.pdf = pow2(smpl.lightDistance) / (emitterCosTheta * areaWorld * totalSamplableEmitters * emitter.numberOfFaces);
    }

    p_isVisible = true;

    const float tMax    = max(0.0, smpl.lightDistance - FLT_EPSILON); 
    // Check if we can see the emitter
    traceRayEXT(
        TLAS,               
        gl_RayFlagsTerminateOnFirstHitEXT, // Ray Flags           
        0xFF,  // Cull Mask         
        1,                  
        0,                  
        1,                  
        worldPosition,      
        RAY_T_MIN,               
        worldInLightDir,    
        tMax,               
        VisibilityPayloadLocation
    );
    
    smpl.isVisible = p_isVisible;
}

void directLighting(SurfaceData surface, vec3 worldPosition, vec3 outLightDir) {
    
    EmitterSample emitterSample;
    
    sampleEmitter(surface, worldPosition, emitterSample);

    if (emitterSample.isVisible && emitterSample.radiance != vec3(0.0)) {
        vec3 halfVector = normalize(outLightDir + emitterSample.inLightDir);

        float receiverCos = cosThetaTangent(emitterSample.inLightDir);

        vec3 bsdf = evaluateBSDF(surface, outLightDir, emitterSample.inLightDir, halfVector);
            
        vec3 contribution = (emitterSample.radiance * bsdf * receiverCos) / emitterSample.pdf;

        p_pathTrace.radiance += contribution;
    }
}

void indirectLighting(SurfaceData surface, vec3 outLightDir, out vec3 inLightDir) {
    float pdf;

    vec3 brdf_multiplier = sampleBSDF(surface, outLightDir, inLightDir, pdf, p_pathTrace.seed);

    if (brdf_multiplier == vec3(0.0)) {
        // No contribution from this surface
        p_pathTrace.done = true;
        return;
    }

    // Apply Russian Roulette termination 
    float russianRouletteProbability = 1.0f;
    if (p_pathTrace.depth > MIN_DEPTH) {
        // Calculate the Russian Roulette probability based on the max component of the throughput
        // to ensure that the path is terminated with a probability proportional to its contribution.
        russianRouletteProbability = maxComponent(p_pathTrace.throughput);

        if (randomFloat(p_pathTrace.seed) > russianRouletteProbability) {
            p_pathTrace.done = true;
            return;
        }
    }

    p_pathTrace.throughput *= brdf_multiplier / russianRouletteProbability;
}

void main() {
    MeshInstanceDescription instance = meshInstances.i[gl_InstanceCustomIndexEXT];
    Material material = materials.m[instance.materialIndex];
    Triangle triangle = getTriangle(instance.indexAddress, instance.vertexAddress, gl_PrimitiveID);

    const vec2 uv = getTextureCoords(triangle, barycentrics) * instance.textureTilingFactor;

    // Tangent, Bi-tangent, Normal (TBN) matrix to transform tangent space to world space
    mat3 tbn = calculateTBN(triangle, mat3(instance.objectToWorld), barycentrics);
    const vec3 surfaceNormal = calculateSurfaceNormal(textures[nonuniformEXT(material.normalMapIndex)], uv, tbn);
    
    vec3 worldNormal = tbn[2];

    SurfaceData surface;
    surface.tbn = tbn;
    surface.albedo = getAlbedo(material, uv, instance.textureTintColor);
    surface.metalness = 0; //getMetalness(material, uv);
    surface.roughness = 1;//getRoughness(material, uv);
    surface.reflectance = calculateReflectance(surface.albedo, surface.metalness);
    surface.specularProbability = calculateSpecularProbability(surface.albedo, surface.metalness, surface.reflectance);
    
    vec3 emission = getEmission(material, uv);

    if (p_pathTrace.depth == 0) {
        p_pathTrace.radiance += emission * p_pathTrace.throughput; 
    }

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

    directLighting(surface, worldPosition, outgoingLightDirection);
    indirectLighting(surface, outgoingLightDirection, incomingLightDirection);
    
    tbn[2] = surfaceNormal;

    // Convert back to world space
    outgoingLightDirection = tangentToWorld(tbn, incomingLightDirection);

    // Update the payload for the next bounce
    p_pathTrace.depth++;
    p_pathTrace.origin = worldPosition + worldNormal * FLT_EPSILON;
    p_pathTrace.direction = outgoingLightDirection;
}