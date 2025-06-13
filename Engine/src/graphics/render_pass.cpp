#include "graphics/render_pass.hpp"

#include <iostream>

namespace PXTEngine {
    RenderPass::RenderPass(Context& context, const VkRenderPassCreateInfo& createInfo,
        const VkAttachmentDescription colorAttachmentDescription,
        const VkAttachmentDescription depthAttachmentDescription,
        std::string name)
        : m_context(context), m_createInfo(createInfo),
		m_colorAttachmentDescription(colorAttachmentDescription),
		m_depthAttachmentDescription(depthAttachmentDescription),
        m_name(name){
        std::cout << "[RenderPass] Creating VkRenderPass: " + m_name << std::endl;
        if (vkCreateRenderPass(m_context.getDevice(), &m_createInfo, nullptr, &m_renderPass) != VK_SUCCESS) {
            throw std::runtime_error("[RenderPass] Failed to create VkRenderPass: " + m_name);
        }
        std::cout << "[RenderPass] VkRenderPass " + m_name + " created successfully." << std::endl;
    }

    RenderPass::~RenderPass() {
        if (m_renderPass != VK_NULL_HANDLE) {
            std::cout << "[RenderPass] Destroying VkRenderPass: " + m_name << std::endl;
            vkDestroyRenderPass(m_context.getDevice(), m_renderPass, nullptr);
            m_renderPass = VK_NULL_HANDLE;
            std::cout << "[RenderPass] VkRenderPass: " + m_name + " destroyed." << std::endl;
        }
    }
}