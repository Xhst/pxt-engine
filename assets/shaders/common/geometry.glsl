#ifndef _GEOMETRY_
#define _GEOMETRY_

#include "math.glsl"

struct Vertex {
    vec4 position;  // Position of the vertex.
    vec4 normal;    // Normal vector for lighting calculations.
    vec4 tangent;   // Tangent vector for lighting calculations.
    vec4 uv;        // Texture coordinates for the vertex.
};

struct Triangle {
    Vertex v0;
    Vertex v1;
    Vertex v2;
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

/*vec3 tangentToWorld(mat3 TBN, vec3 tangentVector) {
    return normalize(TBN * tangentVector);
}

vec3 worldToTangent(mat3 TBN, vec3 worldVector) {
    return normalize(transpose(TBN) * worldVector);
}*/

Triangle getTriangle(uint64_t indexAddress, uint64_t vertexAddress, uint faceIndex) {
    IndexBuffer indices = IndexBuffer(indexAddress);
    VertexBuffer vertices = VertexBuffer(vertexAddress);

    // Retrieve the indices of the triangle being hit.
    uint i0 = indices.i[faceIndex * 3 + 0];
    uint i1 = indices.i[faceIndex * 3 + 1];
    uint i2 = indices.i[faceIndex * 3 + 2];

    Triangle triangle;
    // Retrieve the vertices of the triangle using the indices.
    triangle.v0 = vertices.v[i0];
    triangle.v1 = vertices.v[i1];
    triangle.v2 = vertices.v[i2];

    return triangle;
}

float calculateTriangleArea(Triangle triangle) {
    return 0.5 * length(cross(
        triangle.v1.position.xyz - triangle.v0.position.xyz,
        triangle.v2.position.xyz - triangle.v0.position.xyz
    ));
}

vec2 getTextureCoords(Triangle triangle, vec2 barycentrics) {
    return barycentricLerp(
        triangle.v0.uv.xy,
        triangle.v1.uv.xy,
        triangle.v2.uv.xy,
        barycentrics
    );
}

vec3 getPosition(Triangle triangle, vec2 barycentrics) {
    return barycentricLerp(
        triangle.v0.position.xyz,
        triangle.v1.position.xyz,
        triangle.v2.position.xyz,
        barycentrics
    );
}

vec3 getNormal(Triangle triangle, vec2 barycentrics) {
    return normalize(barycentricLerp(
        triangle.v0.normal.xyz,
        triangle.v1.normal.xyz,
        triangle.v2.normal.xyz,
        barycentrics
    ));
}

vec4 getTangent(Triangle triangle, vec2 barycentrics) {
    return normalize(barycentricLerp(
        triangle.v0.tangent,
        triangle.v1.tangent,
        triangle.v2.tangent,
        barycentrics
    ));
}

mat3 calculateTBN(Triangle triangle, mat3 objectToWorld, vec2 barycentrics) {
	vec3 objectNormal = getNormal(triangle, barycentrics);
	vec4 objectTangent = getTangent(triangle, barycentrics);

    vec3 worldNormal = normalize(objectToWorld * objectNormal);
    vec3 worldTanget = normalize(objectToWorld * objectTangent.xyz);
    float handedness = objectTangent.w;

    // normal
    vec3 N = worldNormal;

    // Gram–Schmidt process
    vec3 T = normalize(worldTanget - dot(worldNormal, worldTanget) * worldNormal);

    // bitangent
    vec3 B = normalize(cross(N, T) * handedness);

    return mat3(T, B, N);
}

vec3 tangentToWorld(const mat3 tbn, const vec3 tangentVector) {
    return tbn * tangentVector;
}

vec3 worldToTangent(const mat3 tbn, const vec3 worldVector) {
    return inverse(tbn) * worldVector;
}

#endif 
