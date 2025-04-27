#pragma once

#include "graphics/resources/vk_image.hpp"


namespace PXTEngine {

	/**
	 * @class Texture2D
	 * @brief Represents a Vulkan Texture and its associated resources.
	 *
	 * This class encapsulates the creation and management of a Vulkan texture, including its view and sampler.
	 * It extends the Image class to provide specific functionality for 2D textures.
	 */
	class Texture2D : public VulkanImage {
	public:
		static Unique<Texture2D> create(const ResourceId& id, const Image::Info& info, const Buffer& buffer, VkFormat format = VK_FORMAT_R8G8B8A8_SRGB);

		Texture2D(Context& context, const ResourceId& id, const Image::Info& info, const Buffer& buffer, VkFormat format = VK_FORMAT_R8G8B8A8_SRGB);

	private:

		/**
		 * @brief Creates a texture image.
		 *
		 * This function creates a Vulkan image and copies the pixel data from the provided buffer to the image.
		 * It also transitions the image layout to be used as a texture.
		 *
		 * @param info The texture information, including width, height, channels
		 * @param buffer The buffer containing the pixel data
		 */
		void createTextureImage(const Image::Info& info, const Buffer& buffer);

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