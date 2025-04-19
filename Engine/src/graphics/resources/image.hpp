#pragma once

#include "graphics/resources/buffer.hpp"

namespace PXTEngine {

	/**
	 * @class Image
	 * @brief Represents a Vulkan image and its associated resources.
	 *
	 * This class encapsulates the creation and management of a Vulkan image, including its view and sampler.
	 * It is a generic class that can be used for different types of images (e.g., 2D, 3D, cubemaps).
	 * 
	 * It can be extended to create specific types of images (e.g., Texture2D, Texture3D, etc.).
	 */
	class Image {
	public:
		Image(Context& context, VkFormat format);
		Image(Context& context, const VkImageCreateInfo& imageInfo, VkMemoryPropertyFlags memoryFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		virtual ~Image();

		Image(const Image&) = delete;
		Image& operator=(const Image&) = delete;
		Image(Image&&) = delete;
		Image& operator=(Image&&) = delete;

		VkImage getVkImage() { return m_vkImage; }
		const VkImageView getImageView() { return m_imageView; }
		const VkSampler getImageSampler() { return m_sampler; }
		const VkFormat getImageFormat() { return m_imageFormat; }

		Image& createImageView(const VkImageViewCreateInfo& viewInfo);
		Image& createSampler(const VkSamplerCreateInfo& samplerInfo);

	protected:
		Context& m_context;

		VkFormat m_imageFormat;

		VkImage m_vkImage; // the raw image pixels
		VkDeviceMemory m_imageMemory; // the memory occupied by the image
		VkImageView m_imageView; // an abstraction to view the same raw image in different "ways"
		VkSampler m_sampler; // an abstraction (and tool) to help fragment shader pick the right color and
									// apply useful transformations (e.g. bilinear filtering, anisotropic filtering etc.)
	};
}