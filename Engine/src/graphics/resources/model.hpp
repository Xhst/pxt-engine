#pragma once

#include "graphics/context/context.hpp"

#include "core/memory.hpp"
#include "utils/hash_func.hpp"

#include "graphics/resources/buffer.hpp"

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
    class Model {
    public:
        /**
         * @struct Vertex
         * 
         * @brief Represents a single vertex with position, color, normal, and texture coordinates.
         */
        struct Vertex {
            glm::vec3 position{};  // Position of the vertex.
            glm::vec3 color{};     // Color of the vertex.
            glm::vec3 normal{};    // Normal vector for lighting calculations.
            glm::vec2 uv{};        // Texture coordinates.
            
            /**
             * @brief Retrieves the binding descriptions for vertex input.
             * 
             * @return A vector of VkVertexInputBindingDescription.
             */
            static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();

            /**
             * @brief Retrieves the attribute descriptions for vertex input.
             * 
             * @return A vector of VkVertexInputAttributeDescription.
             */
            static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
       
            bool operator==(const Vertex& other) const {
                return position == other.position && color == other.color && normal == other.normal && uv == other.uv;
            }
        };

        /**
         * @struct Builder
         * 
         * @brief Helps in constructing a Model by loading vertices and indices from a file.
         */
        struct Builder {
            std::vector<Vertex> vertices{};  // List of vertices in the model.
            std::vector<uint32_t> indices{}; // List of indices for indexed rendering.

            /**
             * @brief Loads a model from an OBJ file.
             * 
             * @param filepath The path to the model file.
             */
            void loadModel(const std::string& filepath);
        };

        Model(Context& context, const Model::Builder& builder);
        ~Model();

        Model(const Model&) = delete;
        Model& operator=(const Model&) = delete;

        /**
         * @brief Creates a Model from a file.
         * 
         * @param context Reference to the Vulkan context.
         * @param filepath Path to the model file.
         * 
         * @return A unique pointer to the created Model.
         */
        static Unique<Model> createModelFromFile(Context& context, const std::string& filepath);

        /**
         * @brief Binds the model's vertex and index buffers to a command buffer.
         * 
         * @param commandBuffer The Vulkan command buffer.
         */
        void bind(VkCommandBuffer commandBuffer);
        
        /**
         * @brief Draws the model using the bound buffers.
         * 
         * @param commandBuffer The Vulkan command buffer.
         */
        void draw(VkCommandBuffer commandBuffer);

    private:
        /**
         * @brief Creates and allocates vertex buffers.
         * 
         * @param vertices The list of vertices to store in the buffer.
         */
        void createVertexBuffers(const std::vector<Vertex>& vertices);

        /**
         * @brief Creates and allocates index buffers.
         * 
         * @param indices The list of indices to store in the buffer.
         */
        void createIndexBuffers(const std::vector<uint32_t>& indices);

        Context& m_context;

        Unique<Buffer> m_vertexBuffer;
        uint32_t m_vertexCount;

        bool m_hasIndexBuffer = false;
        Unique<Buffer> m_indexBuffer;
        uint32_t m_indexCount;
    };
}


template <>
struct std::hash<PXTEngine::Model::Vertex> {
	size_t operator()(PXTEngine::Model::Vertex const &vertex) const noexcept {
		size_t seed = 0;

		PXTEngine::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
		return seed;
	}
};
