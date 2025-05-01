#pragma once

#include "graphics/context/context.hpp"

#include "core/memory.hpp"
#include "resources/types/mesh.hpp"

#include "graphics/resources/vk_buffer.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL

#include <vector>

namespace PXTEngine {

    class VulkanMesh : public Mesh {
    public:
        /**
         * @brief Retrieves the binding descriptions for vertex input.
         *
         * @return A vector of VkVertexInputBindingDescription.
         */
        static std::vector<VkVertexInputBindingDescription> getVertexBindingDescriptions();

        /**
         * @brief Retrieves the attribute descriptions for vertex input.
         *
         * @return A vector of VkVertexInputAttributeDescription.
         */
        static std::vector<VkVertexInputAttributeDescription> getVertexAttributeDescriptions();

        static Unique<VulkanMesh> create(std::vector<Mesh::Vertex>& vertices, 
            std::vector<uint32_t>& indices, Shared<Material> material);

        VulkanMesh(Context& context, std::vector<Mesh::Vertex>& vertices, 
            std::vector<uint32_t>& indices, Shared<Material> material);

        ~VulkanMesh() override;

        VulkanMesh(const VulkanMesh&) = delete;
        VulkanMesh& operator=(const VulkanMesh&) = delete;

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

        const std::vector<Vertex>& getVertices() const override {
            return m_vertices;
        }

        const std::vector<uint32_t>& getIndices() const override {
            return m_indices;
        }

        Type getType() const override {
            return Type::Model;
        }

        Shared<Material> getMaterial() override {
            return m_material;
        }

        void setMaterial(Shared<Material> material) override {
            m_material = material;
        }

		void setTilingFactor(float tilingFactor) override {
			m_tilingFactor = tilingFactor;
		}

		float getTilingFactor() const override {
			return m_tilingFactor;
		}

        Type getType() {
            return Type::Mesh;
        }

    private:
        /**
         * @brief Creates and allocates vertex buffers.
         */
        void createVertexBuffers();

        /**
         * @brief Creates and allocates index buffers.
         */
        void createIndexBuffers();

        Context& m_context;

        std::vector<Mesh::Vertex>& m_vertices;
        std::vector<uint32_t>& m_indices;
		Shared<Material> m_material;
		float m_tilingFactor = 1.0f;

        Unique<VulkanBuffer> m_vertexBuffer;
        uint32_t m_vertexCount;

        bool m_hasIndexBuffer = false;
        Unique<VulkanBuffer> m_indexBuffer;
        uint32_t m_indexCount;
    };
}
