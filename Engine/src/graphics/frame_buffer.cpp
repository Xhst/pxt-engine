#include "graphics/frame_buffer.hpp"
#include "core/logger.hpp"

#include <iostream>

namespace PXTEngine {
    FrameBuffer::FrameBuffer(Context& context,
        VkFramebufferCreateInfo& createInfo,
        std::string name,
        Shared<VulkanImage> colorAttachment,
        Shared<VulkanImage> depthAttachment)
        : m_context(context),
        m_createInfo(createInfo),
        m_name(name),
        m_colorAttachment(colorAttachment),
        m_depthAttachment(depthAttachment) {

        PXT_DEBUG("Creating VkFrameBuffer: {}", m_name);

        if (!m_colorAttachment) {
            throw std::runtime_error("[FrameBuffer] Color attachment cannot be null for FrameBuffer: " + m_name);
        }

        if (createInfo.pAttachments == nullptr || createInfo.attachmentCount == 0) {
            throw std::runtime_error("[FrameBuffer] No attachments specified for FrameBuffer: " + m_name);
        }

        if (vkCreateFramebuffer(m_context.getDevice(), &m_createInfo, nullptr, &m_FrameBuffer) != VK_SUCCESS) {
            throw std::runtime_error("[FrameBuffer] Failed to create VkFrameBuffer: " + m_name);
        }
        PXT_DEBUG("VkFrameBuffer {} created successfully.", m_name);
    }

    FrameBuffer::~FrameBuffer() {
        if (m_FrameBuffer != VK_NULL_HANDLE) {
            PXT_DEBUG("Destroying VkFrameBuffer: {}", m_name);
            vkDestroyFramebuffer(m_context.getDevice(), m_FrameBuffer, nullptr);
            m_FrameBuffer = VK_NULL_HANDLE;
            PXT_DEBUG("VkFrameBuffer {} destroyed.", m_name);
        }
    }
}