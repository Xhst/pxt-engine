#pragma once

#include "graphics/buffer.hpp"

namespace PXTEngine {
	class Image {
	public:
		Image(const std::string filename, Device& device);
		~Image();
		
		Image(const Image&) = delete;
		Image& operator=(const Image&) = delete;
		Image(Image&&) = delete;
		Image& operator=(Image&&) = delete;

		void createTextureImage(const char* filename);

		VkImageView getImageView() { return m_imageView; }
		VkSampler getImageSampler() { return m_sampler; }
		VkImageLayout getImageLayout() { return m_imageLayout; }

	private:
		void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
		void createTextureImageView();
		void createTextureSampler();

		Device& m_device;

		VkImage m_image; // the raw image pixels
		VkDeviceMemory m_imageMemory; // the memory occupied by the image
		VkImageView m_imageView; // an abstraction to view the same raw image in different "ways"
		VkSampler m_sampler; // an abstraction (and tool) to help fragment shader pick the right color and
		VkFormat m_imageFormat; 
		VkImageLayout m_imageLayout;
	};
}