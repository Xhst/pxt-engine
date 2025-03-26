#include "graphics/image.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <stdexcept>


namespace PXTEngine {
	Image::Image(const char* filename, Device& device) : m_device{ device } {
		createTextureImage(filename);
	}
	Image::~Image() {
		vkDestroyImage(m_device.getDevice(), m_textureImage, nullptr);
		vkFreeMemory(m_device.getDevice(), m_textureImageMemory, nullptr);
	}

	void Image::createTextureImage(const char* filename) {
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load(filename, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		VkDeviceSize imageSize = texWidth * texHeight * 4;

		if (!pixels) {
			throw std::runtime_error("failed to load texture image!");
		}
		
		// create a staging buffer visible to the host and copy the pixels to it
		Unique<Buffer> stagingBuffer = createUnique<Buffer>(
			m_device,
			imageSize,
			1,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		);
		stagingBuffer->map(imageSize);
		stagingBuffer->writeToBuffer(pixels, imageSize, 0);
		stagingBuffer->unmap();

		stbi_image_free(pixels);

		// create an empty vkImage
		createImage(texWidth, texHeight,
					VK_FORMAT_R8G8B8A8_SRGB,
					VK_IMAGE_TILING_OPTIMAL,
					// we want the image to be a transfer destination and sampled to be used in the shaders
					VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
					m_textureImage, m_textureImageMemory);
		
		// we now change the layout of the image for better destination copy performance (VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		m_device.transitionImageLayout(m_textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		// then we copy the contents of the image (that were inside the stagingBuffer) into the vkImage
		m_device.copyBufferToImage(stagingBuffer->getBuffer(), m_textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

		// finally, we change the image layout again to be accessed from the shaders (VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		m_device.transitionImageLayout(m_textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

	void Image::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // Not usable by the GPU and the very first transition will discard the texels.
															 // We don't care about the texels now that the image is empty, we will change
															 // the layout later and then copy the pixels to this vkImage.
		imageInfo.usage = usage;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT; // MSAA makes no sense here
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // we want this image to be used only by one queue family (in this case the graphics queue)
		imageInfo.flags = 0; // optional

		m_device.createImageWithInfo(imageInfo, properties, image, imageMemory);
	}
}