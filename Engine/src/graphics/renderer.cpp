#include "graphics/renderer.hpp"

namespace PXTEngine {

    Renderer::Renderer(Window& window, Context& context) : m_window{window}, m_context{context} {
        recreateSwapChain();
        createCommandBuffers();
    }

    Renderer::~Renderer() { 
        freeCommandBuffers(); 
    }

    void Renderer::recreateSwapChain() {
        auto extent = m_window.getExtent();
        while (extent.width == 0 || extent.height == 0) {
            extent = m_window.getExtent();
            glfwWaitEvents();
        }
        vkDeviceWaitIdle(m_context.getDevice());

        if (m_swapChain == nullptr) {
            m_swapChain = createUnique<SwapChain>(m_context, extent);
        } else {
            Shared<SwapChain> oldSwapChain = std::move(m_swapChain);
            m_swapChain = createUnique<SwapChain>(m_context, extent, oldSwapChain);

            if (!oldSwapChain->compareSwapFormats(*m_swapChain.get())) {
                throw std::runtime_error("Swap chain image (format, color space, or size) has changed, not handled yet!");
            }
        }
    }

    void Renderer::createCommandBuffers() {
        m_commandBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = m_context.getCommandPool();
        allocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());

        if (vkAllocateCommandBuffers(m_context.getDevice(), &allocInfo, m_commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }

    void Renderer::freeCommandBuffers() {
        vkFreeCommandBuffers(
            m_context.getDevice(),
            m_context.getCommandPool(),
            static_cast<uint32_t>(m_commandBuffers.size()),
            m_commandBuffers.data());
        
        m_commandBuffers.clear();
    }

    VkCommandBuffer Renderer::beginFrame() {
        PXT_ASSERT(!m_isFrameStarted, "Can't call beginFrame while frame is in progress.");

        auto result = m_swapChain->acquireNextImage(&m_currentImageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain();
            return nullptr;
        }

        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        m_isFrameStarted = true;

        auto commandBuffer = getCurrentCommandBuffer();

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        return commandBuffer;
    }

    void Renderer::endFrame() {
        PXT_ASSERT(m_isFrameStarted, "Can't call endFrame while frame is not in progress.");

        auto commandBuffer = getCurrentCommandBuffer();
        
        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }

        auto result = m_swapChain->submitCommandBuffers(&commandBuffer, &m_currentImageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_window.isWindowResized()) {
            m_window.resetWindowResizedFlag();
            recreateSwapChain();
        } else if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }

        m_isFrameStarted = false;
        m_currentFrameIndex = (m_currentFrameIndex + 1) % SwapChain::MAX_FRAMES_IN_FLIGHT;
    }

    void Renderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer) {
        PXT_ASSERT(m_isFrameStarted, "Can't begin render pass when frame is not in progress.");
        PXT_ASSERT(commandBuffer == getCurrentCommandBuffer(), "Can't begin render pass on command buffer from a different frame.");

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_swapChain->getRenderPass();
        renderPassInfo.framebuffer = m_swapChain->getFrameBuffer(m_currentImageIndex);

        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = m_swapChain->getSwapChainExtent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {0.01f, 0.01f, 0.01f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(m_swapChain->getSwapChainExtent().width);
        viewport.height = static_cast<float>(m_swapChain->getSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{{0, 0}, m_swapChain->getSwapChainExtent()};
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    }

    void Renderer::beginRenderPass(VkCommandBuffer commandBuffer, RenderPass& renderPass, FrameBuffer& frameBuffer, VkExtent2D extent) {
        PXT_ASSERT(m_isFrameStarted, "Can't begin render pass when frame is not in progress.");
        PXT_ASSERT(commandBuffer == getCurrentCommandBuffer(), "Can't begin render pass on command buffer from a different frame.");

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass.getHandle();
		renderPassInfo.framebuffer = frameBuffer.getHandle();

		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = extent;

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 1.0f, 1.0f, 1.0f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(extent.width);
        viewport.height = static_cast<float>(extent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{ {0, 0}, extent };
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        // renderPass begins, so the image will be transitioned to the initial layout
		frameBuffer.getColorAttachment()->setImageLayout(renderPass.getColorAttachmentInitialLayout());

        if (frameBuffer.hasDepthAttachment()) {
            frameBuffer.getDepthAttachment()->setImageLayout(renderPass.getDepthAttachmentInitialLayout());
        }
    }

    void Renderer::endRenderPass(VkCommandBuffer commandBuffer, RenderPass& renderPass, FrameBuffer& frameBuffer) {
        PXT_ASSERT(m_isFrameStarted, "Can't call endRenderPass when frame is not in progress.");
        PXT_ASSERT(commandBuffer == getCurrentCommandBuffer(), "Can't end render pass on command buffer from a different frame.");

        vkCmdEndRenderPass(commandBuffer);

		// After the render pass ends, the image will be transitioned to the final layout
        frameBuffer.getColorAttachment()->setImageLayout(renderPass.getColorAttachmentFinalLayout());

        if (frameBuffer.hasDepthAttachment()) {
            frameBuffer.getDepthAttachment()->setImageLayout(renderPass.getDepthAttachmentFinalLayout());
        }
    }

    void Renderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer) {
        PXT_ASSERT(m_isFrameStarted, "Can't call endSwapChainRenderPass when frame is not in progress.");
        PXT_ASSERT(commandBuffer == getCurrentCommandBuffer(), "Can't end render pass on command buffer from a different frame.");

        vkCmdEndRenderPass(commandBuffer);
    }
}