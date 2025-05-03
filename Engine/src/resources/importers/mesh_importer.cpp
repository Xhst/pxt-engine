#include "resources/importers/mesh_importer.hpp"

#include "core/constants.hpp"
#include "core/memory.hpp"
#include "graphics/resources/vk_mesh.hpp"
#include "resources/types/material.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <unordered_map>
#include <vector>

namespace PXTEngine {

	Shared<Mesh> MeshImporter::importObj(ResourceManager& rm, const std::filesystem::path& filePath,
        ResourceInfo* resourceInfo) {

	    std::vector<Mesh::Vertex> vertices{};  // List of vertices in the model.
	    std::vector<uint32_t> indices{}; // List of indices for indexed rendering.

		tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filePath.string().c_str())) {
            throw std::runtime_error(warn + err);
        }

        vertices.clear();
        indices.clear();

        std::unordered_map<Mesh::Vertex, uint32_t> uniqueVertices{};
        for (const auto& shape : shapes) {
            for (const auto& index : shape.mesh.indices) {
                Mesh::Vertex vertex{};

                if (index.vertex_index >= 0) {
                    vertex.position = {
                        attrib.vertices[3 * index.vertex_index + 0],
                        attrib.vertices[3 * index.vertex_index + 1],
                        attrib.vertices[3 * index.vertex_index + 2],
                    };

                    vertex.color = {
                        attrib.colors[3 * index.vertex_index + 0],
                        attrib.colors[3 * index.vertex_index + 1],
                        attrib.colors[3 * index.vertex_index + 2],
                    };
                }

                if (index.normal_index >= 0) {
                    vertex.normal = {
                        attrib.normals[3 * index.normal_index + 0],
                        attrib.normals[3 * index.normal_index + 1],
                        attrib.normals[3 * index.normal_index + 2],
                    };
                }

                if (index.texcoord_index >= 0) {
                    vertex.uv = {

						attrib.texcoords[2 * index.texcoord_index + 0],
	                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                    };
                }

				if (!uniqueVertices.contains(vertex)) {
				    uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
				    vertices.push_back(vertex);
				}
				indices.push_back(uniqueVertices[vertex]);
            }
        }

        // Iterate through triangles and calculate per-triangle tangents and bitangents
        for (size_t i = 0; i < indices.size(); i += 3) {
            Mesh::Vertex& v0 = vertices[indices[i + 0]];
            Mesh::Vertex& v1 = vertices[indices[i + 1]];
            Mesh::Vertex& v2 = vertices[indices[i + 2]];

            glm::vec3 edge1 = v1.position - v0.position;
            glm::vec3 edge2 = v2.position - v0.position;

            glm::vec2 deltaUV1 = v1.uv - v0.uv;
            glm::vec2 deltaUV2 = v2.uv - v0.uv;

            float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

            glm::vec3 tangent;
            tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
            tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
            tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

            tangent = glm::normalize(tangent);

            float handedness =
                (glm::dot(glm::cross(v0.normal, v1.normal), tangent) < 0.0f) ? -1.0f : 1.0f;

            glm::vec4 tangent4 = glm::vec4(tangent, handedness);

            v0.tangent = tangent4;
            v1.tangent = tangent4;
            v2.tangent = tangent4;
        }

		return VulkanMesh::create(vertices, indices);
	}
}