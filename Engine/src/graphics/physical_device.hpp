#pragma once

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

	class PhysicalDevice {
	public:
        PhysicalDevice(Instance& instance, Surface& surface);

        VkPhysicalDevice getDevice() {
            return m_physicalDevice;
        }

        QueueFamilyIndices findQueueFamilies() {
            return findQueueFamiliesForDevice(m_physicalDevice);
        }

        SwapChainSupportDetails querySwapChainSupport() {
            return querySwapChainSupportForDevice(m_physicalDevice);
        }

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
        QueueFamilyIndices findQueueFamiliesForDevice(VkPhysicalDevice device);

        /**
         * @brief Queries the swap chain support details for a physical device.
         *
         * This function queries the swap chain support details for a physical device, including the surface capabilities,
         * formats, and present modes.
         *
         * @param device The physical device to query the swap chain support details for.
         * @return The swap chain support details.
         */
        SwapChainSupportDetails querySwapChainSupportForDevice(VkPhysicalDevice device);

        /**
         * @brief Checks if the required device extensions are supported.
         *
         * This function checks if all the required device extensions are supported by the physical device.
         *
         * @param device The physical device to check.
         * @return true if all extensions are supported, false otherwise.
         */
        bool checkDeviceExtensionSupport(VkPhysicalDevice device);

        Instance& m_instance;
        Surface& m_surface;

        VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
		
	};
}