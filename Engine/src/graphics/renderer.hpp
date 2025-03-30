#pragma once

#include "core/memory.hpp"
#include "graphics/window.hpp"
#include "graphics/swap_chain.hpp"
#include "graphics/context/context.hpp"

#include <cassert>

namespace PXTEngine {

    /**
     * @class Renderer
     * 
     * @brief Manages rendering operations, including swap chain management and command buffer handling.
     * This class encapsulates the logic for rendering to a window using a Vulkan swap chain. It handles the creation
     * and management of command buffers, frame synchronization, and swap chain recreation when necessary.
     */
    class Renderer {
    public:
        /**
         * @brief Constructs a Renderer object.
         * Initializes the renderer by recreating the swap chain and creating command buffers.
         * 
         * @param window The window to render to.
         * @param context The Vulkan context to use for rendering.
         */
        Renderer(Window& window, Context& context);

        /**
         * @brief Destructor for the Renderer class, frees the command buffers.
         */
        ~Renderer();

        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;

        /**
         * @brief Gets the Vulkan render pass associated with the swap chain.
         * 
         * @return The Vulkan render pass.
         */
        VkRenderPass getSwapChainRenderPass() const { return m_swapChain->getRenderPass(); }

        /**
         * @brief Gets the aspect ratio (width/height) of the swap chain extent.
         * 
         * @return The aspect ratio of the swap chain extent.
         */
        float getAspectRatio() const { return m_swapChain->extentAspectRatio(); }

        /**
         * @brief Checks if a frame is currently in progress.
         * 
         * @return True if a frame is in progress, false otherwise.
         */
        bool isFrameInProgress() const { return m_isFrameStarted; }

        /**
         * @brief Gets the current command buffer.
         * 
         * @return The current command buffer.
         * @throws std::runtime_error if called when no frame is in progress.
         */
        VkCommandBuffer getCurrentCommandBuffer() const {
            assert(m_isFrameStarted && "Cannot get command buffer when frame not in progress.");

            return m_commandBuffers[m_currentFrameIndex];
        }

        /**
         * @brief Gets the current frame index.
         * 
         * @return The current frame index.
         * @throws std::runtime_error if called when no frame is in progress.
         */
        int getFrameIndex() const { 
            assert(m_isFrameStarted && "Cannot get frame index when frame not in progress.");
            
            return m_currentFrameIndex; 
        }

        /**
         * @brief Begins a new frame for rendering.
         * Acquires the next swap chain image, begins recording the command buffer, and returns it.
         * Handles swap chain recreation if it becomes out of date.
         * 
         * @return The command buffer for the new frame.
         * @throws std::runtime_error If acquiring the swap chain image or beginning the command buffer fails.
         */
        VkCommandBuffer beginFrame();

        /**
         * @brief Ends the current frame and presents the rendered image.
         * Ends recording the command buffer, submits it for execution, and presents the rendered image.
         * Handles swap chain recreation if it becomes out of date or if the window is resized.
         * 
         * @throws std::runtime_error If ending the command buffer or presenting the swap chain image fails.
         */
        void endFrame();

        /**
         * @brief Begins the swap chain render pass.
         * 
         * @param commandBuffer The command buffer to record the render pass into.
         * 
         * @throws std::runtime_error if called when frame is not in progress or command buffer is from a different frame.
         */
        void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);

        /**
         * @brief Ends the swap chain render pass.
         * 
         * @param commandBuffer The command buffer to record the end of the render pass into.
         * 
         * @throws std::runtime_error if called when frame is not in progress or command buffer is from a different frame.
         */
        void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

    private:
        /**
         * @brief Creates the command buffers used for rendering.
         * 
         * @throws std::runtime_error If command buffer allocation fails.
         */
        void createCommandBuffers();

        /**
         * @brief Frees the allocated command buffers.
         */
        void freeCommandBuffers();

        /**
         * @brief Recreates the swap chain, handles window resizing and initial swap chain creation.
         * 
         * @throws std::runtime_error If the swap chain image format, color space, or size has changed unexpectedly.
         */
        void recreateSwapChain();

        Window& m_window;
        Context& m_context;
        Unique<SwapChain> m_swapChain;
        std::vector<VkCommandBuffer> m_commandBuffers;

        uint32_t m_currentImageIndex;
        int m_currentFrameIndex = 0;
        bool m_isFrameStarted = false;
    };
}