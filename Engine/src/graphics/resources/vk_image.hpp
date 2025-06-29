#pragma once

#include "core/pch.hpp"
#include "core/buffer.hpp"
#include "resources/types/image.hpp"
#include "graphics/context/context.hpp"

namespace PXTEngine {
	static VkFormat pxtToVulkanImageFormat(const ImageFormat format) {
		switch (format) {
		case RGB8_LINEAR:
			return VK_FORMAT_R8G8B8_UNORM;
		case RGBA8_LINEAR:
			return VK_FORMAT_R8G8B8A8_UNORM;
		case RGB8_SRGB:
			return  VK_FORMAT_R8G8B8_SRGB;
		case RGBA8_SRGB:
			return  VK_FORMAT_R8G8B8A8_SRGB;
		}

		return VK_FORMAT_R8G8B8A8_SRGB;
	}

	static ImageFormat vulkanToPxtImageFormat(const VkFormat format){
		switch (format) {
		case VK_FORMAT_R8G8B8_UNORM:
			return RGB8_LINEAR;
		case VK_FORMAT_R8G8B8A8_UNORM:
			return RGBA8_LINEAR;
		case VK_FORMAT_R8G8B8_SRGB:
			return  RGB8_SRGB;
		case VK_FORMAT_R8G8B8A8_SRGB:
			return  RGBA8_SRGB;
		default:
			return RGBA8_SRGB;
		}
	}

	/**
	 * @class VulkanImage
	 * @brief Represents a Vulkan image and its associated resources.
	 *
	 * This class encapsulates the creation and management of a Vulkan image, including its view and sampler.
	 * It is a generic class that can be used for different types of images (e.g., 2D, 3D, cubeMaps).
	 * 
	 * It can be extended to create specific types of images (e.g., Texture2D, Texture3D, etc.).
	 */
	class VulkanImage : public Image {
	public:
		VulkanImage(Context& context, const ImageInfo& info, const Buffer& buffer);
		VulkanImage(Context& context, const VkImageCreateInfo& imageInfo, VkMemoryPropertyFlags memoryFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		~VulkanImage() override;

		VulkanImage(const VulkanImage&) = delete;
		VulkanImage& operator=(const VulkanImage&) = delete;
		VulkanImage(VulkanImage&&) = delete;
		VulkanImage& operator=(VulkanImage&&) = delete;

		uint32_t getWidth() override {
			return m_info.width;
		}

		uint32_t getHeight() override {
			return m_info.height;
		}

		uint16_t getChannels() override {
			return m_info.channels;
		}

		ImageFormat getFormat() override {
			return m_info.format;
		}

		Type getType() const override {
			return Type::Image;
		}

		VkExtent2D getExtent() const {
			return { m_info.width, m_info.height };
		}

		float getAspectRatio() const {
			return static_cast<float>(m_info.width) / static_cast<float>(m_info.height);
		}

		VkImage getVkImage() { return m_vkImage; }
		const VkImageView getImageView() { return m_imageView; }
		const VkSampler getImageSampler() { return m_sampler; }
		const VkFormat getImageFormat() { return m_imageFormat; }

		const VkImageLayout getCurrentLayout() const { return m_currentLayout; }
		void setImageLayout(const VkImageLayout newLayout) { m_currentLayout = newLayout; }

		VulkanImage& createImageView(const VkImageViewCreateInfo& viewInfo);
		VulkanImage& createSampler(const VkSamplerCreateInfo& samplerInfo);

		/**
		 * @brief Transitions the layout of an image.
		 *
		 * This function transitions the layout of an image, which is required when changing the way the image is accessed.
		 * 
		 * @param oldLayout The old layout of the image.
		 * @param newLayout The new layout of the image.
		 * @param subresourceRange The subresource range of the image.
		 * @param sourceStage The source pipeline stage. If not specified, it will be set to VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, which is less efficient.
		 * @param destinationStage The destination pipeline stage. If not specified, it will be set to VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, which is less efficient.
		 */
		void transitionImageLayoutSingleTimeCmd(VkImageLayout newLayout, VkPipelineStageFlags sourceStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, std::optional<VkImageSubresourceRange> subresourceRange = std::nullopt);

		/**
		 * @brief Transitions the layout of an image.
		 *
		 * This function transitions the layout of an image, which is required when changing the way the image is accessed.
		 *
		 * @param commandBuffer The command buffer handle.
		 * @param oldLayout The old layout of the image.
		 * @param newLayout The new layout of the image.
		 * @param subresourceRange The subresource range of the image.
		 * @param sourceStage The source pipeline stage. If not specified, it will be set to VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, which is less efficient.
		 * @param destinationStage The destination pipeline stage. If not specified, it will be set to VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, which is less efficient.
		 */
		void transitionImageLayout(VkCommandBuffer commandBuffer, VkImageLayout newLayout, VkPipelineStageFlags sourceStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, std::optional<VkImageSubresourceRange> subresourceRange = std::nullopt);


	protected:
		Context& m_context;

		VkFormat m_imageFormat;

		ImageInfo m_info;
		VkImage m_vkImage; // the raw image pixels
		VkDeviceMemory m_imageMemory; // the memory occupied by the image
		VkImageView m_imageView; // an abstraction to view the same raw image in different "ways"
		VkSampler m_sampler; // an abstraction (and tool) to help fragment shader pick the right color and
									// apply useful transformations (e.g. bilinear filtering, anisotropic filtering etc.)

		VkImageLayout m_currentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	};
}