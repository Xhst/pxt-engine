#pragma once

#include "graphics/context/context.hpp"
#include "graphics/resources/vk_image.hpp"

// FrameBuffer Wrapper Class
// mainly used to save the FrameBuffer creation info and the
// resources linked to it (attachments)
namespace PXTEngine {
    class FrameBuffer {
    public:
        FrameBuffer(Context& context,
            const VkFramebufferCreateInfo& createInfo,
            std::string name,
            Shared<VulkanImage> colorAttachment,
            Shared<VulkanImage> depthAttachment = nullptr);
        ~FrameBuffer();

        // Disable copy constructor and assignment operator
        FrameBuffer(const FrameBuffer&) = delete;
        FrameBuffer& operator=(const FrameBuffer&) = delete;

        VkFramebuffer getVkFrameBuffer() const { return m_FrameBuffer; }
        const VkFramebufferCreateInfo& getCreateInfo() const { return m_createInfo; }
        const Context& getContext() const { return m_context; }
        const Shared<VulkanImage>& getColorAttachment() const { return m_colorAttachment; }
        const Shared<VulkanImage>& getDepthAttachment() const { return m_depthAttachment; }
        bool hasDepthAttachment() const { return (bool)m_depthAttachment; }

    private:
        Context& m_context;
        std::string m_name;
        VkFramebufferCreateInfo m_createInfo;
        VkFramebuffer m_FrameBuffer = VK_NULL_HANDLE; // Vulkan handle
        Shared<VulkanImage> m_colorAttachment; // Shared pointer to the color attachment image
        Shared<VulkanImage> m_depthAttachment; // Shared pointer to the depth attachment image (optional)
    };
}