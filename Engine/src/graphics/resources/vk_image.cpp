#include "graphics/resources/vk_image.hpp"

#include <format>

namespace PXTEngine {
	VulkanImage::VulkanImage(Context& context, const ImageInfo& info, const Buffer& buffer) :
	m_context(context),
	m_info(info) {
		// set other members as VK_NULL_HANDLE
		m_vkImage = VK_NULL_HANDLE;
		m_imageMemory = VK_NULL_HANDLE;
		m_imageView = VK_NULL_HANDLE;
		m_sampler = VK_NULL_HANDLE;

		switch (info.format)
		{
		case RGB8_LINEAR:
			m_imageFormat = VK_FORMAT_R8G8B8_UNORM;
			break;
		case RGBA8_LINEAR:
			m_imageFormat = VK_FORMAT_R8G8B8A8_UNORM;
			break;
		case RGB8_SRGB:
			m_imageFormat = VK_FORMAT_R8G8B8_SRGB;
			break;
		case RGBA8_SRGB:
			m_imageFormat = VK_FORMAT_R8G8B8A8_SRGB;
		}
	}

	VulkanImage::VulkanImage(Context& context, const VkImageCreateInfo& imageInfo, VkMemoryPropertyFlags memoryFlags)
	: VulkanImage(context, ImageInfo(imageInfo.extent.width, imageInfo.extent.height, 4), Buffer()) {
		// create the image and allocate memory for it
		m_context.createImageWithInfo(imageInfo, memoryFlags, m_vkImage, m_imageMemory);
	}

	VulkanImage::~VulkanImage() {
		vkDestroySampler(m_context.getDevice(), m_sampler, nullptr);
		vkDestroyImageView(m_context.getDevice(), m_imageView, nullptr);

		vkDestroyImage(m_context.getDevice(), m_vkImage, nullptr);
		vkFreeMemory(m_context.getDevice(), m_imageMemory, nullptr);
	}

	VulkanImage& VulkanImage::createImageView(const VkImageViewCreateInfo& viewInfo) {
		if (m_imageView != VK_NULL_HANDLE) {
			vkDestroyImageView(m_context.getDevice(), m_imageView, nullptr);
		}

		m_imageView = m_context.createImageView(viewInfo);

		return *this;
	}

	VulkanImage& VulkanImage::createSampler(const VkSamplerCreateInfo& samplerInfo) {
		if (m_sampler != VK_NULL_HANDLE) {
			vkDestroySampler(m_context.getDevice(), m_sampler, nullptr);
		}

		m_sampler = m_context.createSampler(samplerInfo);

		return *this;
	}

	void VulkanImage::transitionImageLayoutSingleTimeCmd(VkImageLayout newLayout,
		VkPipelineStageFlags sourceStage, VkPipelineStageFlags destinationStage,
		std::optional<VkImageSubresourceRange> subresourceRange) {

		VkCommandBuffer commandBuffer = m_context.beginSingleTimeCommands();

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = m_currentLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; // If you are using the barrier to transfer queue family ownership, then these two fields should be the indices of the queue families. 
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; // They must be set to VK_QUEUE_FAMILY_IGNORED if you don't want to do this (not the default value!).
		barrier.image = m_vkImage;
		if (subresourceRange.has_value()) {
			barrier.subresourceRange = subresourceRange.value();
		}
		else {
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0; // if you are using mipMapping
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0; // if it's an array (1D image)
			barrier.subresourceRange.layerCount = 1;
		}

		// Source layouts (old)
		// Source access mask controls actions that have to be finished on the old layout
		// before it will be transitioned to the new layout
		switch (m_currentLayout) {
		case VK_IMAGE_LAYOUT_UNDEFINED:
			// Image layout is undefined (or does not matter)
			// Only valid as initial layout
			// No flags required, listed only for completeness
			barrier.srcAccessMask = 0;
			break;

		case VK_IMAGE_LAYOUT_PREINITIALIZED:
			// Image is preinitialized
			// Only valid as initial layout for linear images, preserves memory contents
			// Make sure host writes have been finished
			barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			// Image is a color attachment
			// Make sure any writes to the color buffer have been finished
			barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			// Image is a depth/stencil attachment
			// Make sure any writes to the depth/stencil buffer have been finished
			barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			// Image is a transfer source
			// Make sure any reads from the image have been finished
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			break;

		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			// Image is a transfer destination
			// Make sure any writes to the image have been finished
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			// Image is read by a shader
			// Make sure any shader reads from the image have been finished
			barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			break;
		case VK_IMAGE_LAYOUT_GENERAL:
			// Image is used as a general image
			// Make sure any writes to the image have been finished
			barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
			break;
		default:
			// Other source layouts aren't handled (yet)
			throw std::runtime_error("Unsupported old image layout when transitioning");
			break;
		}

		// Target layouts (new)
		// Destination access mask controls the dependency for the new image layout
		switch (newLayout) {
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			// Image will be used as a transfer destination
			// Make sure any writes to the image have been finished
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			// Image will be used as a transfer source
			// Make sure any reads from the image have been finished
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			break;

		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			// Image will be used as a color attachment
			// Make sure any writes to the color buffer have been finished
			barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			// Image layout will be used as a depth/stencil attachment
			// Make sure any writes to depth/stencil buffer have been finished
			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			// Image will be read in a shader (sampler, input attachment)
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			break;
		case VK_IMAGE_LAYOUT_GENERAL:
			// Image will be used as a general image
			// Make sure any writes to the image have been finished
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
			break;
		default:
			// Other source layouts aren't handled (yet)
			throw std::runtime_error("Unsupported new image layout when transitioning");
			break;
		}

		vkCmdPipelineBarrier(
			commandBuffer,
			sourceStage, destinationStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		m_context.endSingleTimeCommands(commandBuffer);

		// save the layout of the image as it's immediatly transitioned
		setImageLayout(newLayout);
	}

	void VulkanImage::transitionImageLayout(VkCommandBuffer commandBuffer, VkImageLayout newLayout, VkPipelineStageFlags sourceStage, VkPipelineStageFlags destinationStage, std::optional<VkImageSubresourceRange> subresourceRange)
	{
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = m_currentLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; // If you are using the barrier to transfer queue family ownership, then these two fields should be the indices of the queue families. 
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; // They must be set to VK_QUEUE_FAMILY_IGNORED if you don't want to do this (not the default value!).
		barrier.image = m_vkImage;
		if (subresourceRange.has_value()) {
			barrier.subresourceRange = subresourceRange.value();
		}
		else {
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0; // if you are using mipMapping
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0; // if it's an array
			barrier.subresourceRange.layerCount = 1;
		}

		// Source layouts (old)
		// Source access mask controls actions that have to be finished on the old layout
		// before it will be transitioned to the new layout
		switch (m_currentLayout) {
		case VK_IMAGE_LAYOUT_UNDEFINED:
			// Image layout is undefined (or does not matter)
			// Only valid as initial layout
			// No flags required, listed only for completeness
			barrier.srcAccessMask = 0;
			break;

		case VK_IMAGE_LAYOUT_PREINITIALIZED:
			// Image is preinitialized
			// Only valid as initial layout for linear images, preserves memory contents
			// Make sure host writes have been finished
			barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			// Image is a color attachment
			// Make sure any writes to the color buffer have been finished
			barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			// Image is a depth/stencil attachment
			// Make sure any writes to the depth/stencil buffer have been finished
			barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			// Image is a transfer source
			// Make sure any reads from the image have been finished
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			break;

		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			// Image is a transfer destination
			// Make sure any writes to the image have been finished
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			// Image is read by a shader
			// Make sure any shader reads from the image have been finished
			barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			break;
		case VK_IMAGE_LAYOUT_GENERAL:
			// Image is used as a general image
			// Make sure any writes to the image have been finished
			barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
			break;
		default:
			// Other source layouts aren't handled (yet)
			throw std::runtime_error("Unsupported old image layout when transitioning");
			break;
		}

		// Target layouts (new)
		// Destination access mask controls the dependency for the new image layout
		switch (newLayout) {
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			// Image will be used as a transfer destination
			// Make sure any writes to the image have been finished
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			// Image will be used as a transfer source
			// Make sure any reads from the image have been finished
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			break;

		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			// Image will be used as a color attachment
			// Make sure any writes to the color buffer have been finished
			barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			// Image layout will be used as a depth/stencil attachment
			// Make sure any writes to depth/stencil buffer have been finished
			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			// Image will be read in a shader (sampler, input attachment)
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			break;
		case VK_IMAGE_LAYOUT_GENERAL:
			// Image will be used as a general image
			// Make sure any writes to the image have been finished
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
			break;
		default:
			// Other source layouts aren't handled (yet)
			throw std::runtime_error("Unsupported new image layout when transitioning");
			break;
		}

		vkCmdPipelineBarrier(
			commandBuffer,
			sourceStage, destinationStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		// save the layout of the image
		// in this case it's more strange
		// essentially we are recording to the command buffer to perfrom
		// the transition, so at the end of the gpu command buffer execution
		// the image will be in the layout of the last transition of the
		// command buffer
		setImageLayout(newLayout);
	}
}
