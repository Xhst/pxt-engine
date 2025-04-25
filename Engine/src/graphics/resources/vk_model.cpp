#include "graphics/resources/vk_model.hpp"

#include "core/error_handling.hpp"

namespace PXTEngine {
    VulkanModel::VulkanModel(Context& context, const ResourceId& id,
                             const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
        : Model(id, vertices, indices), m_context(context) {
        createVertexBuffers(vertices);
        createIndexBuffers(indices);
    }

    VulkanModel::~VulkanModel() = default;

    void VulkanModel::createVertexBuffers(const std::vector<Vertex>& vertices) {
        m_vertexCount = static_cast<uint32_t>(vertices.size());

        PXT_ASSERT(m_vertexCount >= 3, "Vertex count must be at least 3");

        VkDeviceSize bufferSize = sizeof(vertices[0]) * m_vertexCount;

        uint32_t vertexSize = sizeof(vertices[0]);

        VulkanBuffer stagingBuffer{
            m_context,
            vertexSize,
            m_vertexCount, 
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        };

        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void*) vertices.data());

        m_vertexBuffer = createUnique<VulkanBuffer>(
            m_context, 
            vertexSize, 
            m_vertexCount, 
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );

        m_context.copyBuffer(stagingBuffer.getBuffer(), m_vertexBuffer->getBuffer(), bufferSize);
    }

    void VulkanModel::createIndexBuffers(const std::vector<uint32_t>& indices) {
        m_indexCount = static_cast<uint32_t>(indices.size());
        m_hasIndexBuffer = m_indexCount > 0;

        if (!m_hasIndexBuffer) return;

        VkDeviceSize bufferSize = sizeof(indices[0]) * m_indexCount;
        uint32_t indexSize = sizeof(indices[0]);

        VulkanBuffer stagingBuffer{
            m_context,
            indexSize,
            m_indexCount, 
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        };

        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void*) indices.data());

        m_indexBuffer = createUnique<VulkanBuffer>(
            m_context, 
            indexSize, 
            m_indexCount, 
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );

        m_context.copyBuffer(stagingBuffer.getBuffer(), m_indexBuffer->getBuffer(), bufferSize);
    }

    void VulkanModel::draw(VkCommandBuffer commandBuffer) {
        if (m_hasIndexBuffer) {
            vkCmdDrawIndexed(commandBuffer, m_indexCount, 1, 0, 0, 0);
        } else {
            vkCmdDraw(commandBuffer, m_vertexCount, 1, 0, 0);
        }
    }

    void VulkanModel::bind(VkCommandBuffer commandBuffer) {
        VkBuffer buffers[] = {m_vertexBuffer->getBuffer()};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

        if (m_hasIndexBuffer) {
            vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
        }
    }

    std::vector<VkVertexInputBindingDescription> VulkanModel::getVertexBindingDescriptions() {
        std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
        bindingDescriptions[0].binding = 0;
        bindingDescriptions[0].stride = sizeof(Vertex);
        bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescriptions;
    }

    std::vector<VkVertexInputAttributeDescription> VulkanModel::getVertexAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
        
        attributeDescriptions.push_back({0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Model::Vertex, position)});
        attributeDescriptions.push_back({1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Model::Vertex, color)});
        attributeDescriptions.push_back({2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Model::Vertex, normal)});
        attributeDescriptions.push_back({3, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Model::Vertex, tangent) });
        attributeDescriptions.push_back({4, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Model::Vertex, uv)});

        return attributeDescriptions;
    }
}