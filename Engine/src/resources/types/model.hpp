#pragma once

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
     * @class Model
     *
     * @brief Represents a 3D model.
     */
    class Model : public Resource {
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

        Model(const ResourceId& id, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
            : Resource(id, Type::Model), m_vertices(vertices), m_indices(indices) {}

        Model(const Model&) = delete;
        Model& operator=(const Model&) = delete;

        const std::vector<Vertex>& getVertices() const { return m_vertices; }
        const std::vector<uint32_t>& getIndices() const { return m_indices; }

        static Type getStaticType() { return Type::Model; }

    protected:
        std::vector<Vertex> m_vertices;
        std::vector<uint32_t> m_indices;
    };
}

template <>
struct std::hash<PXTEngine::Model::Vertex> {
    size_t operator()(PXTEngine::Model::Vertex const &vertex) const noexcept {
        size_t seed = 0;

        PXTEngine::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.tangent, vertex.uv);
        return seed;
    }
};