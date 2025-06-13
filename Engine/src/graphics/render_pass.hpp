#pragma once

#include "graphics/context/context.hpp"

// RenderPass Wrapper Class
// mainly used to save the render pass creation info and the
// resources linked to it
namespace PXTEngine {
    class RenderPass {
    public:
        RenderPass(Context& context, const VkRenderPassCreateInfo& createInfo, std::string name);
        ~RenderPass();

        // Disable copy constructor and assignment operator for proper resource management
        RenderPass(const RenderPass&) = delete;
        RenderPass& operator=(const RenderPass&) = delete;

        VkRenderPass getVkRenderPass() const { return m_renderPass; }
        const VkRenderPassCreateInfo& getCreateInfo() const { return m_createInfo; }

    private:
        Context& m_context;
        std::string m_name; // Name for logging
        VkRenderPassCreateInfo m_createInfo;
        VkRenderPass m_renderPass = VK_NULL_HANDLE; // renderPass Vulkan handle
    };
}