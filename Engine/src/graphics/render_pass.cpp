#include "graphics/render_pass.hpp"

namespace PXTEngine {
    RenderPass::RenderPass(Context& context, const VkRenderPassCreateInfo& createInfo,
        const VkAttachmentDescription colorAttachmentDescription,
        const VkAttachmentDescription depthAttachmentDescription,
        std::string name)
        : m_context(context), m_createInfo(createInfo),
		m_colorAttachmentDescription(colorAttachmentDescription),
		m_depthAttachmentDescription(depthAttachmentDescription),
        m_name(name){
        PXT_DEBUG("Creating VkRenderPass: {}", m_name);
        if (vkCreateRenderPass(m_context.getDevice(), &m_createInfo, nullptr, &m_renderPass) != VK_SUCCESS) {
            throw std::runtime_error("[RenderPass] Failed to create VkRenderPass: " + m_name);
        }
        PXT_DEBUG("VkRenderPass {} created successfully.", m_name);
    }

    RenderPass::~RenderPass() {
        if (m_renderPass != VK_NULL_HANDLE) {
            PXT_DEBUG("Destroying VkRenderPass: {}", m_name);
            vkDestroyRenderPass(m_context.getDevice(), m_renderPass, nullptr);
            m_renderPass = VK_NULL_HANDLE;
            PXT_DEBUG("VkRenderPass: {} destroyed.", m_name);
        }
    }
}