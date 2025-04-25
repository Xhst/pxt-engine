#include "graphics/resources/vk_image.hpp"


namespace PXTEngine {
	VulkanImage::VulkanImage(Context& context, const ResourceId& id, const Image::Info& info, const Buffer& buffer, VkFormat format)
	: Image(id, info, buffer),
	m_context(context),
	m_imageFormat(format) {
		// set other members as VK_NULL_HANDLE
		m_vkImage = VK_NULL_HANDLE;
		m_imageMemory = VK_NULL_HANDLE;
		m_imageView = VK_NULL_HANDLE;
		m_sampler = VK_NULL_HANDLE;
	}

	VulkanImage::VulkanImage(Context& context, const ResourceId& id, const VkImageCreateInfo& imageInfo, VkMemoryPropertyFlags memoryFlags)
	: VulkanImage(context, id, Image::Info(imageInfo.extent.width, imageInfo.extent.height, 4), Buffer(), imageInfo.format) {
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