#pragma once

#include "core/memory.hpp"
#include "resources/resource.hpp"
#include "utils/hash_func.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <glm/glm.hpp>

#include <vector>

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
        struct Vertex {
            glm::vec3 position{};  // Position of the vertex.
            glm::vec3 color{};     // Color of the vertex.
            glm::vec3 normal{};    // Normal vector for lighting calculations.
            glm::vec4 tangent{};   // Tangent vector for lighting calculations.
            glm::vec2 uv{};        // Texture coordinates.

            bool operator==(const Vertex& other) const {
                return position == other.position
                    && color == other.color
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

        PXTEngine::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.tangent, vertex.uv);
        return seed;
    }
};