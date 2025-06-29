#pragma once

#include "core/pch.hpp"
#include "graphics/context/context.hpp"
#include "resources/types/mesh.hpp"
#include "graphics/resources/vk_buffer.hpp"

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

        static Unique<VulkanMesh> create(std::vector<Mesh::Vertex>& vertices, std::vector<uint32_t>& indices);

        VulkanMesh(Context& context, std::vector<Mesh::Vertex>& vertices, std::vector<uint32_t>& indices);

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

        const uint32_t getVertexCount() const override {
			return m_vertexCount;
        }

        const uint32_t getIndexCount() const override {
			return m_indexCount;
        }

		VkDeviceAddress getVertexBufferDeviceAddress() const {
            return m_vertexBuffer->getDeviceAddress();
		}

        VkDeviceAddress getIndexBufferDeviceAddress() const {
            return m_indexBuffer->getDeviceAddress();
        }

        Type getType() const override {
            return Type::Mesh;
        }

    private:
        /**
         * @brief Creates and allocates vertex buffers.
         */
        void createVertexBuffers(std::vector<Mesh::Vertex>& vertices);

        /**
         * @brief Creates and allocates index buffers.
         */
        void createIndexBuffers(std::vector<uint32_t>& indices);

        Context& m_context;

		float m_tilingFactor = 1.0f;

        Unique<VulkanBuffer> m_vertexBuffer;
        uint32_t m_vertexCount;

        bool m_hasIndexBuffer = false;
        Unique<VulkanBuffer> m_indexBuffer;
        uint32_t m_indexCount;
    };
}
