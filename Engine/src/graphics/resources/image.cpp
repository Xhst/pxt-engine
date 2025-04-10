#include "graphics/resources/image.hpp"


namespace PXTEngine {
	Image::Image(Context& context, VkFormat format) : m_context(context), m_imageFormat(format) {
		// set other members as VK_NULL_HANDLE
		m_vkImage = VK_NULL_HANDLE;
		m_imageMemory = VK_NULL_HANDLE;
		m_imageView = VK_NULL_HANDLE;
		m_sampler = VK_NULL_HANDLE;
	}

	Image::Image(Context& context, const VkImageCreateInfo& imageInfo, VkMemoryPropertyFlags memoryFlags) : Image(context, imageInfo.format) {
		// create the image and allocate memory for it
		m_context.createImageWithInfo(imageInfo, memoryFlags, m_vkImage, m_imageMemory);
	}

	Image::~Image() {
		vkDestroySampler(m_context.getDevice(), m_sampler, nullptr);
		vkDestroyImageView(m_context.getDevice(), m_imageView, nullptr);

		vkDestroyImage(m_context.getDevice(), m_vkImage, nullptr);
		vkFreeMemory(m_context.getDevice(), m_imageMemory, nullptr);
	}

	Image& Image::createImageView(const VkImageViewCreateInfo& viewInfo) {
		if (m_imageView != VK_NULL_HANDLE) {
			vkDestroyImageView(m_context.getDevice(), m_imageView, nullptr);
		}

		m_imageView = m_context.createImageView(viewInfo);

		return *this;
	}

	Image& Image::createSampler(const VkSamplerCreateInfo& samplerInfo) {
		if (m_sampler != VK_NULL_HANDLE) {
			vkDestroySampler(m_context.getDevice(), m_sampler, nullptr);
		}

		m_sampler = m_context.createSampler(samplerInfo);

		return *this;
	}
}