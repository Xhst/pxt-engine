#pragma once

#include <string>
#include <vector>

#include "graphics/instance.hpp"
#include "graphics/window.hpp"
#include "graphics/instance.hpp"
#include "graphics/surface.hpp"

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

        /**
         * @brief Constructor for the Device class.
         *
         * This constructor initializes the Vulkan device, including creating the instance,
         * setting up the debug messenger, creating the surface, picking a physical device,
         * creating the logical device, and creating the command pool.
         *
         * @param window The window to create the surface from.
         */
        Device(Window& window, Instance& instance, Surface& surface);

        /**
         * @brief Destructor for the Device class.
         *
         * This destructor destroys the Vulkan device, including destroying the command pool,
         * logical device, debug messenger (if enabled), surface, and instance.
         */
        ~Device();

        // Not copyable or movable
        Device(const Device&) = delete;
        void operator=(const Device&) = delete;
        Device(Device&&) = delete;
        Device &operator=(Device&&) = delete;

        VkCommandPool getCommandPool() { return m_commandPool; }
        VkDevice getDevice() { return m_device; }
        VkSurfaceKHR getSurface() { return m_surface.getSurface(); }
        VkQueue graphicsQueue() { return m_graphicsQueue; }
        VkQueue presentQueue() { return m_presentQueue; }

        VkInstance getVkInstance() {return m_instance.getVkInstance();}
        VkPhysicalDevice getPhysicalDevice() {return m_physicalDevice;}

        SwapChainSupportDetails getSwapChainSupport() {
            return querySwapChainSupport(m_physicalDevice);
        }

        /**
         * @brief Finds a suitable memory type for a buffer or image.
         *
         * This function finds a suitable memory type for a buffer or image, given the type filter
         * and the required memory properties.
         *
         * @param typeFilter The type filter.
         * @param properties The required memory properties.
         * @return The memory type index.
         */
        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

        /**
         * @brief Finds the queue families for the physical device.
         *
         * This function finds the graphics and present queue families for the physical device.
         *
         * @return The queue family indices.
         */
        QueueFamilyIndices findPhysicalQueueFamilies() {
            return findQueueFamilies(m_physicalDevice);
        }

        /**
         * @brief Gets the graphics and present queue family indices.
         *
         * This function returns the graphics and present queue family indices for the physical device.
         *
         * @return The graphics and present queue family indices.
         */
        int32_t getGraphicsQueueFamily() {
            return findPhysicalQueueFamilies().graphicsFamily;
        }

        /**
         * @brief Gets the present queue family index.
         *
         * This function returns the present queue family index for the physical device.
         *
         * @return The present queue family index.
         */
        int32_t getPresentQueueFamily() {
            return findPhysicalQueueFamilies().presentFamily;
        }

        /**
         * @brief Finds a supported format for an image.
         *
         * This function finds a supported format for an image, given a list of candidate formats,
         * the tiling mode, and the required features.
         *
         * @param candidates The list of candidate formats.
         * @param tiling The tiling mode.
         * @param features The required features.
         * @return The supported format.
         */
        VkFormat findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, 
                                     VkFormatFeatureFlags features);


        /* ----------------------- Buffer Helper Functions ----------------------- */

        /**
         * @brief Creates a buffer.
         *
         * This function creates a buffer with the given size, usage, and memory properties.
         *
         * @param size The size of the buffer.
         * @param usage The usage of the buffer.
         * @param properties The memory properties of the buffer.
         * @param buffer The buffer handle.
         * @param bufferMemory The buffer memory handle.
         */
        void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, 
                          VkBuffer &buffer, VkDeviceMemory &bufferMemory);

        /**
         * @brief Begins single-time commands.
         *
         * This function begins a command buffer for single-time commands.
         *
         * @return The command buffer handle.
         */
        VkCommandBuffer beginSingleTimeCommands();

        /**
         * @brief Ends single-time commands.
         *
         * This function ends a command buffer for single-time commands and submits it to the graphics queue.
         *
         * @param commandBuffer The command buffer handle.
         */
        void endSingleTimeCommands(VkCommandBuffer commandBuffer);

        /**
         * @brief Copies data from a source buffer to a destination buffer.
         *
         * This function copies data from a source buffer to a destination buffer using a command buffer.
         *
         * @param srcBuffer The source buffer handle.
         * @param dstBuffer The destination buffer handle.
         * @param size The size of the data to copy.
         */
        void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

        /**
         * @brief Copies data from a buffer to an image.
         *
         * This function copies data from a buffer to an image using a command buffer.
         *
         * @param buffer The source buffer handle.
         * @param image The destination image handle.
         * @param width The width of the image.
         * @param height The height of the image.
         * @param layerCount The number of image layers.
         */
        void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount = 1);

        /**
         * @brief Creates an image with the given create info and memory properties.
         *
         * This function creates an image and allocates memory for it.
         *
         * @param imageInfo The image create info.
         * @param properties The memory properties.
         * @param image The image handle.
         * @param imageMemory The image memory handle.
         */
        void createImageWithInfo(const VkImageCreateInfo &imageInfo, VkMemoryPropertyFlags properties,
                                 VkImage &image, VkDeviceMemory &imageMemory);

        /**
         * @brief Creates an image view for an image.
         *
         * This function creates an image view for an image, which is used to access the image data.
         *
         * @param image The image handle.
         * @param format The format of the image.
         * @return The image view handle.
         */
        VkImageView createImageView(VkImage image, VkFormat format);

        /**
         * @brief Transitions the layout of an image.
         *
         * This function transitions the layout of an image, which is required when changing the way the image is accessed.
         *
         * @param image The image handle.
         * @param format The format of the image.
         * @param oldLayout The old layout of the image.
         * @param newLayout The new layout of the image.
         */
        void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

        VkPhysicalDeviceProperties properties;

    private:
        
        /**
         * @brief Picks a suitable physical device.
         *
         * This function enumerates the available physical devices and selects one that
         * supports the required features and extensions.
         */
        void pickPhysicalDevice();

        /**
         * @brief Creates a logical device.
         *
         * This function creates a logical device, which is used to interact with the physical device.
         * It also creates the graphics and present queues.
         */
        void createLogicalDevice();

        /**
         * @brief Creates a command pool.
         *
         * This function creates a command pool, which is used to allocate command buffers.
         */
        void createCommandPool();



        /* ---------------------------- Helper functions ---------------------------- */

        /**
         * @brief Checks if a physical device is suitable.
         *
         * This function checks if a physical device supports the required features and extensions.
         *
         * @param device The physical device to check.
         * @return true if the device is suitable, false otherwise.
         */
        bool isDeviceSuitable(VkPhysicalDevice device);

        /**
         * @brief Finds the queue families for a physical device.
         *
         * This function finds the graphics and present queue families for a physical device.
         *
         * @param device The physical device to find the queue families for.
         * @return The queue family indices.
         */
        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

        /**
         * @brief Checks if the required device extensions are supported.
         *
         * This function checks if all the required device extensions are supported by the physical device.
         *
         * @param device The physical device to check.
         * @return true if all extensions are supported, false otherwise.
         */
        bool checkDeviceExtensionSupport(VkPhysicalDevice device);

        /**
         * @brief Queries the swap chain support details for a physical device.
         *
         * This function queries the swap chain support details for a physical device, including the surface capabilities,
         * formats, and present modes.
         *
         * @param device The physical device to query the swap chain support details for.
         * @return The swap chain support details.
         */
        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

        Window& m_window;
        Instance& m_instance;
		Surface& m_surface;

        VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
        
        VkCommandPool m_commandPool;

        VkDevice m_device;
        
        VkQueue m_graphicsQueue;
        VkQueue m_presentQueue;
    };

}