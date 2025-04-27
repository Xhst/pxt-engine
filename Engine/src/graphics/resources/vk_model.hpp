#pragma once

#include "graphics/context/context.hpp"

#include "core/memory.hpp"
#include "resources/types/model.hpp"

#include "graphics/resources/vk_buffer.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL

#include <vector>

namespace PXTEngine {

    class VulkanModel : public Model {
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

        static Unique<VulkanModel> create(const ResourceId& id,
            const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

        VulkanModel(Context& context, const ResourceId& id,
                    const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

        ~VulkanModel();

        VulkanModel(const VulkanModel&) = delete;
        VulkanModel& operator=(const VulkanModel&) = delete;

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
        void createVertexBuffers(const std::vector<Model::Vertex>& vertices);

        /**
         * @brief Creates and allocates index buffers.
         * 
         * @param indices The list of indices to store in the buffer.
         */
        void createIndexBuffers(const std::vector<uint32_t>& indices);

        Context& m_context;

        Unique<VulkanBuffer> m_vertexBuffer;
        uint32_t m_vertexCount;

        bool m_hasIndexBuffer = false;
        Unique<VulkanBuffer> m_indexBuffer;
        uint32_t m_indexCount;
    };
}
