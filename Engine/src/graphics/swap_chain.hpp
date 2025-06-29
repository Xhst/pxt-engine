#pragma once

#include "core/pch.hpp"
#include "graphics/context/context.hpp"

namespace PXTEngine {

    /**
     * @class SwapChain
     * 
     * The Swap Chain is a series of images that are waiting to be presented to the screen.
     * The general purpose of the swap chain is to synchronize the presentation of images 
     * with the refresh rate of the screen.
     * 
     * @brief Manages the Vulkan swap chain for rendering.
     */
    class SwapChain {
    public:

        /**
         * @brief Maximum number of frames in flight.
         * 
         * The number of frames in flight is the maximum number of frames that can be rendered
         * simultaneously. This value is used to determine the number of synchronization objects
         * required for rendering.
         * 
         * @note This value is set to 2 for double buffering, it can be increased for triple buffering.
         */
        static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

        SwapChain(Context& context, VkExtent2D windowExtent);
        SwapChain(Context& context, VkExtent2D windowExtent, Shared<SwapChain> previous);
        ~SwapChain();

        SwapChain(const SwapChain&) = delete;
        SwapChain& operator=(const SwapChain&) = delete;

        /**
         * @brief Retrieves a framebuffer at the specified index.
         * 
         * @param index Index of the framebuffer.
         * 
         * @return VkFramebuffer at the given index.
         */
        VkFramebuffer getFrameBuffer(int index) {
            return m_swapChainFramebuffers[index];
        }

        /**
         * @brief Gets the render pass associated with the swap chain.
         * 
         * @return The Vulkan render pass.
         */
        VkRenderPass getRenderPass() { return m_renderPass; }

        /**
         * @brief Gets the image view at the specified index.
         * 
         * @param index Index of the image view.
         * 
         * @return VkImageView at the given index.
         */
        VkImageView getImageView(int index) {
            return m_swapChainImageViews[index];
        }

        /**
         * @brief Gets the number of images in the swap chain.
         * 
         * @return The number of images in the swap chain.
         */
        size_t imageCount() { return m_swapChainImages.size(); }

        /**
         * @brief Gets the swap chain image format.
         * 
         * @return The swap chain image format.
         */
        VkFormat getSwapChainImageFormat() { return m_swapChainImageFormat; }

        /**
         * @brief Gets the swap chain depth format.
         * 
         * @return The swap chain depth format.
         */
        VkExtent2D getSwapChainExtent() { return m_swapChainExtent; }

        /**
         * @brief Gets the width of the swap chain.
         * 
         * @return The width of the swap chain.
         */
        uint32_t width() { return m_swapChainExtent.width; }

        /**
         * @brief Gets the height of the swap chain.
         * 
         * @return The height of the swap chain.
         */
        uint32_t height() { return m_swapChainExtent.height; }

        /**
         * @brief Gets the aspect ratio of the swap chain (width / height).
         * 
         * @return The aspect ratio of the swap chain.
         */
        float extentAspectRatio() {
            return static_cast<float>(m_swapChainExtent.width) /
                   static_cast<float>(m_swapChainExtent.height);
        }

        /**
         * @brief Acquires the next available swap chain image.
         *
         * This function waits for the current frame's fence to ensure the GPU has finished
         * processing previous work. Then, it gets the next available image in the swap chain.
         *
         * @param imageIndex Pointer to store the acquired image index.
         * @return Vulkan result indicating success or failure.
         */
        VkResult acquireNextImage(uint32_t *imageIndex);

        /**
         * @brief Submits command buffers for rendering.
         * 
         * This function waits for the appropriate synchronization objects before submitting
         * the command buffer to the graphics queue. It then signals the render-finished semaphore
         * and presents the rendered image to the swap chain.
         * 
         * @param buffers Pointer to the command buffers.
         * @param imageIndex Pointer to the index of the image to render to.
         * 
         * @return Vulkan result indicating success or failure.
         */
        VkResult submitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex);
        
        /**
         * @brief This function compares the image and depth formats of the swap chain with another swap chain.
         * 
         * @param swapChain The swap chain to compare formats with.
         * 
         * @return True if the formats match, false otherwise.
         */
        bool compareSwapFormats(const SwapChain& swapChain) const {
            return m_swapChainImageFormat == swapChain.m_swapChainImageFormat &&
                   m_swapChainDepthFormat == swapChain.m_swapChainDepthFormat;
        }

    private:
        /**
         * @brief Initializes the swap chain and related resources.
         * 
         * This method sets up the swap chain and all required components,
         * including image views, render pass, depth resources, framebuffers,
         * and synchronization objects.
         */
        void init();

        /**
         * @brief Creates the Vulkan swap chain.
         * 
         * The swap chain is responsible for handling multiple image buffers used for
         * rendering frames before presenting them to the screen. It determines the
         * number of images, format, extent, and presentation mode.
         * 
         * @throws std::runtime_error If the swap chain creation fails.
         */
        void createSwapChain();

        /**
         * @brief Creates image views for swap chain images.
         * 
         * Image views are necessary to interpret image data in the swap chain
         * and allow shaders to access these images properly.
         * 
         * @throws std::runtime_error If image view creation fails.
         */
        void createImageViews();

        /**
         * @brief Creates depth buffer resources.
         * 
         * The depth buffer is used for depth testing to ensure proper rendering
         * of 3D objects based on their distance from the camera.
         * 
         * @throws std::runtime_error If depth resource creation fails.
         */
        void createDepthResources();

        /**
         * @brief Creates a render pass.
         * 
         * A render pass defines the sequence of operations in which rendering
         * will take place, specifying color and depth attachments and their load/store
         * operations.
         * 
         * @throws std::runtime_error If render pass creation fails.
         */
        void createRenderPass();

        /**
         * @brief Creates framebuffers for rendering.
         * 
         * Framebuffers connect the render pass to the swap chain images, allowing
         * rendered images to be stored before presentation.
         * 
         * @throws std::runtime_error If framebuffer creation fails.
         */
        void createFramebuffers();

        /**
         * @brief Creates synchronization objects.
         * 
         * This includes semaphores and fences to ensure proper synchronization
         * between CPU and GPU, preventing race conditions while rendering frames.
         * 
         * @throws std::runtime_error If synchronization object creation fails.
         */
        void createSyncObjects();

        /**
         * @brief Chooses the optimal surface format for the swap chain.
         * 
         * Selects the best format from available options, preferring VK_FORMAT_B8G8R8A8_SRGB 
         * with VK_COLOR_SPACE_SRGB_NONLINEAR_KHR.
         * 
         * @param availableFormats A list of available surface formats.
         * @return The chosen VkSurfaceFormatKHR.
         */
        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

        /**
         * @brief Chooses the best presentation mode for the swap chain.
         * 
         * Prefers VK_PRESENT_MODE_MAILBOX_KHR if available for low-latency triple buffering.
         * Defaults to VK_PRESENT_MODE_FIFO_KHR (V-Sync) if no better option exists.
         * 
         * @param availablePresentModes A list of available presentation modes.
         * @return The chosen VkPresentModeKHR.
         */
        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

        /**
         * @brief Determines the swap chain extent (resolution of images in the swap chain).
         * 
         * If the current extent is not defined, it is calculated based on the window size 
         * within the allowed min/max bounds.
         * 
         * @param capabilities The surface capabilities that define min/max extent.
         * @return The chosen VkExtent2D.
         */
        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

        // Index of the current frame, this value is between 0 and MAX_FRAMES_IN_FLIGHT
        size_t m_currentFrame = 0;

        VkExtent2D m_swapChainExtent;
        
        VkFormat m_swapChainImageFormat;
        VkFormat m_swapChainDepthFormat;


        std::vector<VkFramebuffer> m_swapChainFramebuffers;
        VkRenderPass m_renderPass;

        std::vector<VkImage> m_depthImages;
        std::vector<VkDeviceMemory> m_depthImageMemorys;
        std::vector<VkImageView> m_depthImageViews;
        std::vector<VkImage> m_swapChainImages;
        std::vector<VkImageView> m_swapChainImageViews;

        Context& m_context;
        VkExtent2D m_windowExtent;

        VkSwapchainKHR m_swapChain;
        Shared<SwapChain> m_oldSwapChain;

        std::vector<VkSemaphore> m_imageAvailableSemaphores;
        std::vector<VkSemaphore> m_renderFinishedSemaphores;
        std::vector<VkFence> m_inFlightFences;
    };

}