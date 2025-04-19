#pragma once

#include "graphics/resources/image.hpp"

namespace PXTEngine {

	/**
	 * @class Texture2D
	 * @brief Represents a Vulkan Texture and its associated resources.
	 *
	 * This class encapsulates the creation and management of a Vulkan texture, including its view and sampler.
	 * It extends the Image class to provide specific functionality for 2D textures.
	 */
	class Texture2D : public Image {
	public:
		Texture2D(const std::string& filename, Context& context, VkFormat format = VK_FORMAT_R8G8B8A8_SRGB);

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
		void createImage(uint32_t width, uint32_t height, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

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
	};
}