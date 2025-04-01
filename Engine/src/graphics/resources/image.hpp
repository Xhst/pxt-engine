#pragma once

#include "graphics/resources/buffer.hpp"

namespace PXTEngine {

	/**
	 * @class Image
	 * @brief Represents a Vulkan image and its associated resources.
	 *
	 * This class encapsulates the creation and management of a Vulkan image, including its view and sampler.
	 * It provides methods to load an image from a file and create the necessary Vulkan resources.
	 */
	class Image {
	public:
		Image(const std::string filename, Context& context);
		~Image();

		Image(const Image&) = delete;
		Image& operator=(const Image&) = delete;
		Image(Image&&) = delete;
		Image& operator=(Image&&) = delete;

		const VkImageView getImageView() { return m_textureImageView; }
		const VkSampler getImageSampler() { return m_textureSampler; }

	private:
		/**
		 * @brief Loads an image from a file and copies it to a Vulkan image.
		 * 
		 * This function uses the stb_image library to load an image from a file and copy it to a Vulkan image.
		 * The image is loaded in RGBA format and the Vulkan image is created with the VK_FORMAT_R8G8B8A8_SRGB format.
		 * 
		 * @param filename The path to the image file.
		 * 
		 * @throw std::runtime_error if the image file cannot be loaded.
		 */
		void createTextureImage(const char* filename);

		/**
		 * @brief Creates a Vulkan image.
		 */
		void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
		
		/**
		 * @brief Creates an image view.
		 * 
		 * An image view is a way to interpret the image data. 
		 * It describes how to access the image and which part of the image to access.
		 */
		void createTextureImageView();

		/**
		 * @brief Creates a texture sampler.
		 * 
		 * A texture sampler is a set of parameters that control how textures are read and sampled by the GPU.
		 */
		void createTextureSampler();

		Context& m_context;

		VkImage m_textureImage; // the raw image pixels
		VkDeviceMemory m_textureImageMemory; // the memory occupied by the image
		VkImageView m_textureImageView; // an abstraction to view the same raw image in different "ways"
		VkSampler m_textureSampler; // an abstraction (and tool) to help fragment shader pick the right color and
									// apply useful transformations (e.g. bilinear filtering, anisotropic filtering etc.)
	};
}