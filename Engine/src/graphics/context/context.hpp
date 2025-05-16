#pragma once

#include <optional>

#include "graphics/context/instance.hpp"
#include "graphics/context/surface.hpp"
#include "graphics/context/physical_device.hpp"
#include "graphics/context/logical_device.hpp"

// IMGUI
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imconfig.h"
#include "imgui_internal.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

namespace PXTEngine {

	/**
	 * @class Context
	 * 
	 * @brief Manages the Vulkan context, including instance, surface, physical device, and logical device.
	 * 
	 * This class is responsible for creating and managing the Vulkan context, including the instance,
	 * surface, physical device, and logical device. It also provides helper functions for buffer and image operations.
	 */
	class Context {
	public:
		Context(Window& window);
		~Context();		
	
		VkInstance getInstance() { return m_instance.getVkInstance(); }
		Window& getWindow() { return m_window; }
		VkSurfaceKHR getSurface() { return m_surface.getSurface(); }
		VkPhysicalDevice getPhysicalDevice() { return m_physicalDevice.getDevice(); }
		VkDevice getDevice() { return m_device.getDevice(); }

		VkCommandPool getCommandPool() { return m_commandPool; }

		VkPhysicalDeviceProperties getPhysicalDeviceProperties() {
			return m_physicalDevice.properties;
		}

		SwapChainSupportDetails getSwapChainSupport() {
			return m_physicalDevice.querySwapChainSupport();
		}

		QueueFamilyIndices findPhysicalQueueFamilies() {
			return m_physicalDevice.findQueueFamilies();
		}

		bool getSupportedDepthFormat(VkFormat* format);

		VkQueue getGraphicsQueue() { return m_device.getGraphicsQueue(); }
		VkQueue getPresentQueue() { return m_device.getPresentQueue(); }

		/* ----------------------- Buffer Helper Functions ----------------------- */

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
						  VkBuffer& buffer, VkDeviceMemory& bufferMemory);

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
		void createImageWithInfo(const VkImageCreateInfo& imageInfo, VkMemoryPropertyFlags properties,
								 VkImage& image, VkDeviceMemory& imageMemory);

		/**
		* @brief Creates an image view for an image.
		*
		* This function creates an image view for an image, which is used to access the image data.
		*
		* @param viewInfo The image view create info.
		* @return The image view handle.
		*/
		VkImageView createImageView(const VkImageViewCreateInfo& viewInfo);

		/**
		* @brief Creates a sampler for an image.
		* 
		* This function creates a sampler for an image, which is used to sample the image data.
		* 
		* @param samplerInfo The sampler create info.
		* @return The sampler handle.
		*/
		VkSampler createSampler(const VkSamplerCreateInfo& samplerInfo);

		/**
		 * @brief Transitions the layout of an image.
		 *
		 * This function transitions the layout of an image, which is required when changing the way the image is accessed.
		 *
		 * @param image The image handle.
		 * @param format The format of the image.
		 * @param oldLayout The old layout of the image.
		 * @param newLayout The new layout of the image.
		 * @param subresourceRange The subresource range of the image.
		 * @param sourceStage The source pipeline stage. If not specified, it will be set to VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, which is less efficient.
		 * @param destinationStage The destination pipeline stage. If not specified, it will be set to VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, which is less efficient.
		 */
		void transitionImageLayoutSingleTimeCmd(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, VkPipelineStageFlags sourceStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, std::optional<VkImageSubresourceRange> subresourceRange = std::nullopt);

		/**
		 * @brief Transitions the layout of an image.
		 *
		 * This function transitions the layout of an image, which is required when changing the way the image is accessed.
		 *
		 * @param commandBuffer The command buffer handle.
		 * @param image The image handle.
		 * @param format The format of the image.
		 * @param oldLayout The old layout of the image.
		 * @param newLayout The new layout of the image.
		 * @param subresourceRange The subresource range of the image.
		 * @param sourceStage The source pipeline stage. If not specified, it will be set to VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, which is less efficient.
		 * @param destinationStage The destination pipeline stage. If not specified, it will be set to VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, which is less efficient.
		 */
		void transitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, VkPipelineStageFlags sourceStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, std::optional<VkImageSubresourceRange> subresourceRange = std::nullopt);

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
		VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling,
				               		 VkFormatFeatureFlags features);

		/* ----------------------- End Buffer Helper Functions ---------------------- */
		
	private:
		/**
		 * @brief Creates a command pool.
		 *
		 * This function creates a command pool, which is used to allocate command buffers.
		 */
		void createCommandPool();

		Window& m_window;
		Instance m_instance;
		Surface m_surface;
		PhysicalDevice m_physicalDevice;
		LogicalDevice m_device;

		VkCommandPool m_commandPool;

	};
}