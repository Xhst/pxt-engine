#pragma once

#include <string>
#include <vector>

#include "graphics/window.hpp"

namespace PXTEngine {

    /**
     * @struct SwapChainSupportDetails
     * @brief Stores details about the swap chain support for a given Vulkan surface.
     *
     * This structure is used to query and store information about the swap chain capabilities
     * of a physical device for a specific surface. It contains details necessary for creating
     * an optimal swap chain configuration.
     */
    struct SwapChainSupportDetails {
        /**
         * @brief Specifies the surface capabilities.
         *
         * This field contains details about the swap chain's constraints and capabilities,
         * including:
         * - The minimum and maximum number of images the swap chain can support.
         * - The current width and height of the surface.
         * - The supported transforms (e.g., rotation, mirroring).
         * - Supported image usage flags (e.g., rendering, storage, transfer).
         *
         * It is retrieved using `vkGetPhysicalDeviceSurfaceCapabilitiesKHR`.
         */
        VkSurfaceCapabilitiesKHR capabilities;
        
        /**
         * @brief A list of supported surface formats.
         *
         * Each format specifies a combination of:
         * - A color format (e.g., `VK_FORMAT_B8G8R8A8_UNORM`), which determines the color depth and arrangement.
         * - A color space (e.g., `VK_COLOR_SPACE_SRGB_NONLINEAR_KHR`), which defines how colors are interpreted.
         *
         * The application needs to select a format compatible with both the swap chain and the rendering pipeline.
         * This list is retrieved using `vkGetPhysicalDeviceSurfaceFormatsKHR`.
         */
        std::vector<VkSurfaceFormatKHR> formats;
        
        /**
         * @brief A list of supported presentation modes.
         *
         * Presentation modes determine how images are presented to the screen. Common modes include:
         * - `VK_PRESENT_MODE_IMMEDIATE_KHR`: Frames are presented immediately, possibly causing screen tearing.
         * - `VK_PRESENT_MODE_FIFO_KHR`: Uses a queue (V-Sync), ensuring no tearing but with potential input latency.
         * - `VK_PRESENT_MODE_MAILBOX_KHR`: A triple-buffering approach reducing latency while avoiding tearing.
         * - `VK_PRESENT_MODE_FIFO_RELAXED_KHR`: Similar to FIFO but allows late frames to be presented immediately.
         *
         * The application selects the best mode based on performance and latency requirements.
         * This list is retrieved using `vkGetPhysicalDeviceSurfacePresentModesKHR`.
         */
        std::vector<VkPresentModeKHR> presentModes;
    };

    /**
     * @struct QueueFamilyIndices
     * @brief Stores indices of queue families needed for Vulkan operations.
     *
     * This structure helps in identifying queue families that support graphics and presentation.
     * Vulkan devices can have multiple queue families, and different operations (such as rendering
     * and presentation) may require separate queue families.
     */
    struct QueueFamilyIndices {
        /**
         * @brief Index of the queue family that supports graphics operations.
         *
         * This queue family must support `VK_QUEUE_GRAPHICS_BIT`, meaning it can be used
         * for rendering commands.
         */
        uint32_t graphicsFamily;

        /**
         * @brief Index of the queue family that supports presentation to a surface.
         *
         * This queue family must be capable of presenting rendered images to a Vulkan surface.
         * It is determined using `vkGetPhysicalDeviceSurfaceSupportKHR`.
         */
        uint32_t presentFamily;

        /**
         * @brief Indicates if a valid graphics queue family index has been found.
         *
         * Set to `true` if `graphicsFamily` has been assigned a valid queue index.
         */
        bool graphicsFamilyHasValue = false;

        /**
         * @brief Indicates if a valid presentation queue family index has been found.
         *
         * Set to `true` if `presentFamily` has been assigned a valid queue index.
         */
        bool presentFamilyHasValue = false;

        /**
         * @brief Checks if both required queue families have been found.
         *
         * @return `true` if both graphics and presentation queue families are valid.
         */
        bool isComplete() {
            return graphicsFamilyHasValue && presentFamilyHasValue;
        }
    };

    class Device {
       public:
#ifdef NDEBUG
        const bool enableValidationLayers = false;
#else
        const bool enableValidationLayers = true;
#endif

        Device(Window &window);
        ~Device();

        // Not copyable or movable
        Device(const Device&) = delete;
        void operator=(const Device&) = delete;
        Device(Device&&) = delete;
        Device &operator=(Device&&) = delete;

        VkCommandPool getCommandPool() { return m_commandPool; }
        VkDevice getDevice() { return m_device; }
        VkSurfaceKHR surface() { return m_surface; }
        VkQueue graphicsQueue() { return m_graphicsQueue; }
        VkQueue presentQueue() { return m_presentQueue; }

        VkInstance getVkInstance() {return m_instance;}
        VkPhysicalDevice getPhysicalDevice() {return m_physicalDevice;}

        SwapChainSupportDetails getSwapChainSupport() {
            return querySwapChainSupport(m_physicalDevice);
        }
        uint32_t findMemoryType(uint32_t typeFilter,
                                VkMemoryPropertyFlags properties);
        QueueFamilyIndices findPhysicalQueueFamilies() {
            return findQueueFamilies(m_physicalDevice);
        }
        int32_t getGraphicsQueueFamily() {
            return findPhysicalQueueFamilies().graphicsFamily;
        }
        int32_t getPresentQueueFamily() {
            return findPhysicalQueueFamilies().presentFamily;
        }
        VkFormat findSupportedFormat(const std::vector<VkFormat> &candidates,
                                     VkImageTiling tiling,
                                     VkFormatFeatureFlags features);

        // Buffer Helper Functions
        void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                          VkMemoryPropertyFlags properties, VkBuffer &buffer,
                          VkDeviceMemory &bufferMemory);
        VkCommandBuffer beginSingleTimeCommands();
        void endSingleTimeCommands(VkCommandBuffer commandBuffer);
        void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer,
                        VkDeviceSize size);
        void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width,
                               uint32_t height, uint32_t layerCount = 1);

        void createImageWithInfo(const VkImageCreateInfo &imageInfo,
                                 VkMemoryPropertyFlags properties,
                                 VkImage &image, VkDeviceMemory &imageMemory);

        VkImageView createImageView(VkImage image, VkFormat format);

        void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

        VkPhysicalDeviceProperties properties;

       private:
        void createInstance();
        void setupDebugMessenger();
        void createSurface();
        void pickPhysicalDevice();
        void createLogicalDevice();
        void createCommandPool();

        // helper functions
        bool isDeviceSuitable(VkPhysicalDevice device);
        std::vector<const char*> getRequiredExtensions();
        bool checkValidationLayerSupport();
        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
        void populateDebugMessengerCreateInfo(
            VkDebugUtilsMessengerCreateInfoEXT &createInfo);
        void hasGflwRequiredInstanceExtensions();
        bool checkDeviceExtensionSupport(VkPhysicalDevice device);
        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

        VkInstance m_instance;
        VkDebugUtilsMessengerEXT m_debugMessenger;
        VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
        Window &m_window;
        VkCommandPool m_commandPool;

        VkDevice m_device;
        VkSurfaceKHR m_surface;
        VkQueue m_graphicsQueue;
        VkQueue m_presentQueue;

        const std::vector<const char*> m_validationLayers = {"VK_LAYER_KHRONOS_validation"};
        const std::vector<const char*> m_deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    };

}