#pragma once

#include "graphics/buffer.hpp"

namespace PXTEngine {
	class Image {
	public:
		Image(const char* filename, Device& device);
		~Image();

		void createTextureImage(const char* filename);

	private:
		void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

		Device& m_device;

		VkImage m_textureImage;
		VkDeviceMemory m_textureImageMemory;
	};
}