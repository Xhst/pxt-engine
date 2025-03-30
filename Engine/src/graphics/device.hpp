#pragma once

#include <vector>

#include "graphics/window.hpp"
#include "graphics/instance.hpp"
#include "graphics/surface.hpp"
#include "graphics/physical_device.hpp"

namespace PXTEngine {

    class Device {
       public:

        /**
         * @brief Constructor for the Device class.
         *
         * This constructor initializes the Vulkan device, including creating the instance,
         * setting up the debug messenger, creating the surface, picking a physical device,
         * creating the logical device, and creating the command pool.
         *
         * @param window The window to create the surface from.
         */
        Device(Window& window, Instance& instance, Surface& surface, PhysicalDevice& physicalDevice);

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
        VkPhysicalDevice getPhysicalDevice() {return m_physicalDevice.getDevice();}

		VkPhysicalDeviceProperties getPhysicalDeviceProperties() {
            return m_physicalDevice.properties;
		}

        SwapChainSupportDetails getSwapChainSupport() {
            return m_physicalDevice.querySwapChainSupport();
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
            return m_physicalDevice.findQueueFamilies();
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

    private:

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


        

        Window& m_window;
        Instance& m_instance;
        Surface& m_surface;
        PhysicalDevice& m_physicalDevice;
        
        VkCommandPool m_commandPool;

        VkDevice m_device;
        
        VkQueue m_graphicsQueue;
        VkQueue m_presentQueue;
    };

}