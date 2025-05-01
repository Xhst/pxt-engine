#pragma once

#include "core/memory.hpp"
#include "resources/resource.hpp"
#include "resources/types/material.hpp"
#include "utils/hash_func.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <glm/glm.hpp>

#include <vector>

namespace PXTEngine {

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

        virtual const std::vector<Vertex>& getVertices() const = 0;
        virtual const std::vector<uint32_t>& getIndices() const  = 0;

		virtual Shared<Material> getMaterial() = 0;
        virtual void setMaterial(Shared<Material> material) = 0;

		virtual float getTilingFactor() const = 0;
		virtual void setTilingFactor(float factor) = 0;

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