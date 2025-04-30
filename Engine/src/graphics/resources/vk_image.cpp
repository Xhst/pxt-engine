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
}
