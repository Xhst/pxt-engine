#include "graphics/image.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <stdexcept>


namespace PXTEngine {
	Image::Image(const std::string filename, Device& device) : m_device{ device } {
		createTextureImage(filename.c_str());
		createTextureImageView();
		createTextureSampler();
	}
	Image::~Image() {
		vkDestroySampler(m_device.getDevice(), m_textureSampler, nullptr);
		vkDestroyImageView(m_device.getDevice(), m_textureImageView, nullptr);

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

	void Image::createTextureImageView() {
		m_textureImageView = m_device.createImageView(m_textureImage, VK_FORMAT_R8G8B8A8_SRGB);
	}

	// view https://vulkan-tutorial.com/Texture_mapping/Image_view_and_sampler for info (there are a lot)
	void Image::createTextureSampler() {
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR; // nearest is pixelated, linear is smooth
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT; // what to do when image is smaller than surface
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT; // the repeat mode is probably the most common mode,
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT; // because it can be used to tile textures like floors and walls.
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = m_device.properties.limits.maxSamplerAnisotropy;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK; // which color to use when sampling outside the image borders (only if address mode is clamp to border)
		samplerInfo.unnormalizedCoordinates = VK_FALSE; // false uses [0,1) coordinates. Useful for giving same coords to different resolution textures.
		samplerInfo.compareEnable = VK_FALSE;		  // This is mainly used for percentage-closer filtering on shadow maps.
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS; // https://developer.nvidia.com/gpugems/gpugems/part-ii-lighting-and-shadows/chapter-11-shadow-map-antialiasing
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR; // mipMapping stuff for later
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;
		
		if (vkCreateSampler(m_device.getDevice(), &samplerInfo, nullptr, &m_textureSampler) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture sampler!");
		}
	}
}