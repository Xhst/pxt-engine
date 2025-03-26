#pragma once

#include "graphics/buffer.hpp"

namespace PXTEngine {
	class Image {
	public:
		Image(const std::string filename, Device& device);
		~Image();

		/**
		 * @brief Deleted copy constructor.
		 */
		Image(const Image&) = delete;

		/**
		 * @brief Deleted copy assignment operator.
		 */
		Image& operator=(const Image&) = delete;
		Image(Image&&) = delete;
		Image& operator=(Image&&) = delete;

		void createTextureImage(const char* filename);
		const VkImageView getImageView() { return m_textureImageView; }
		const VkSampler getImageSampler() { return m_textureSampler; }

	private:
		void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
		void createTextureImageView();
		void createTextureSampler();

		Device& m_device;

		VkImage m_textureImage; // the raw image pixels
		VkDeviceMemory m_textureImageMemory; // the memory occupied by the image
		VkImageView m_textureImageView; // an abstraction to view the same raw image in different "ways"
		VkSampler m_textureSampler; // an abstraction (and tool) to help fragment shader pick the right color and
									// apply useful transformations (e.g. bilinear filtering, anisotropic filtering etc.)
	};
}