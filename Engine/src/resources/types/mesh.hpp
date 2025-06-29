#pragma once

#include "core/pch.hpp"
#include "resources/resource.hpp"
#include "utils/hash_func.hpp"

namespace PXTEngine {

	/**
	 * @struct MeshInfo
	 *
	 * @brief Represents additional information about a mesh resource.
	 * This struct can be used to store metadata or other relevant information about the mesh.
	 */
	struct MeshInfo : public ResourceInfo {
	};

	/**
	 * @class Mesh
	 *
	 * @brief Represents a 3D mesh resource.
	 * This class is used to store vertex and index data for rendering.
	 */
    class Mesh : public Resource {
    public:

        /**
         * @struct Vertex
         *
         * @brief Represents a single vertex with position, color, normal, tangent, and texture coordinates.
         */
        struct alignas(16) Vertex {
            glm::vec4 position{};  // Position of the vertex. (1 unused)
            glm::vec4 normal{};    // Normal vector for lighting calculations. (1 unused)
            glm::vec4 tangent{};   // Tangent vector for lighting calculations.
            glm::vec4 uv{};        // Texture coordinates. (2 unused)

            bool operator==(const Vertex& other) const {
                return position == other.position
                    && normal == other.normal
                    && tangent == other.tangent
                    && uv == other.uv;
            }
        };

        virtual const uint32_t getVertexCount() const = 0;
        virtual const uint32_t getIndexCount() const  = 0;

        static Type getStaticType() { return Type::Mesh; }
    };
}

template <>
struct std::hash<PXTEngine::Mesh::Vertex> {
    size_t operator()(PXTEngine::Mesh::Vertex const& vertex) const noexcept {
        size_t seed = 0;

        PXTEngine::hashCombine(seed, vertex.position, vertex.normal, vertex.tangent, vertex.uv);
        return seed;
    }
};