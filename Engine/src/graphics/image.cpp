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
		m_device.transitionImageLayout(
			m_textureImage, 
			VK_FORMAT_R8G8B8A8_SRGB, 
			VK_IMAGE_LAYOUT_UNDEFINED, 
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
		);

		// then we copy the contents of the image (that were inside the stagingBuffer) into the vkImage
		m_device.copyBufferToImage(
			stagingBuffer->getBuffer(), 
			m_textureImage, 
			static_cast<uint32_t>(texWidth), 
			static_cast<uint32_t>(texHeight)
		);

		// finally, we change the image layout again to be accessed from the shaders (VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		m_device.transitionImageLayout(
			m_textureImage, 
			VK_FORMAT_R8G8B8A8_SRGB, 
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		);
	}

	void Image::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
		 VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {
			
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent = { width, height, 1 };
		
		// The number of mip levels (1 means no mip mapping).
		// A full mipmap chain would be 1 + log2(max(width, height, depth)) levels
		imageInfo.mipLevels = 1;

		// The number of layers in the image (1 means that it's a regular image).
		// Values > 1 are used for array textures (e.g., for cube maps, 3D texture atlases, or layered framebuffers).
		imageInfo.arrayLayers = 1;

		// Specifies the format of the image (color depth, channels, etc.).
		// The format affects memory usage and compatibility
		imageInfo.format = format;

		// Specifies how image data is stored in memory.
		// VK_IMAGE_TILING_LINEAR: Texels are laid out in row-major order (similar to CPU memory).
		// VK_IMAGE_TILING_OPTIMAL: Texels are laid out in an implementation-defined order for optimal access for GPU.
		imageInfo.tiling = tiling;

		// The initialLayout specifies the layout of the image data on the GPU at the start.
		// VK_IMAGE_LAYOUT_UNDEFINED: Not usable by the GPU and the very first transition will discard the texels.
		// We don't care about the texels now that the image is empty, we will change the layout later and then copy
		// the pixels to this vkImage.
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		// Specifies the way the image will be used.
		// Multiple values can be combined using bitwise OR.
		// https://registry.khronos.org/vulkan/specs/latest/man/html/VkImageUsageFlagBits.html#_description
		imageInfo.usage = usage;

		// Specifies the number of samples per pixel (used for anti-aliasing).
		// We don't use MSAA here so we leave it at one sample.
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT; 

		// Defines how the image is shared between queues.
		// We want this image to be used only by one queue family (in this case the graphics queue)
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		imageInfo.flags = 0; // optional

		m_device.createImageWithInfo(imageInfo, properties, image, imageMemory);
	}

	void Image::createTextureImageView() {
		m_textureImageView = m_device.createImageView(m_textureImage, VK_FORMAT_R8G8B8A8_SRGB);
	}

	void Image::createTextureSampler() {
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

		// magFilter & minFilter determine how the texture is sampled when scaled up (mag) or down (min).
		// VK_FILTER_LINEAR: linear interpolation (blurry but smooth)
		// VK_FILTER_LINEAR: nearest neighbor interpolation (pixelated appearance)
		samplerInfo.magFilter = VK_FILTER_LINEAR; 
		samplerInfo.minFilter = VK_FILTER_LINEAR;

		// addressMode (U-V-W): determine what happens when texture coordinates go beyond the image boundaries.
		// Example: https://vulkan-tutorial.com/images/texture_addressing.png
		// VK_SAMPLER_ADDRESS_MODE_REPEAT: repeats the texture
		// VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT: flips every repeat
		// VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE: stretches edge texels
		// VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER: uses a specified border color
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT; 
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT; 
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT; 

		// enable anisotropic filtering, which improves texture quality at oblique angles.
		// https://en.wikipedia.org/wiki/Anisotropic_filtering
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = m_device.getPhysicalDeviceProperties().limits.maxSamplerAnisotropy;

		// which color to use when sampling outside the image borders (only if address mode is clamp to border)
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

		// VK_FALSE: Uses normalized texture coordinates [0,1) range, useful for giving same coords to different resolution textures.
		// VK_TRUE: Uses actual texel coordinates [0, texWidth) and [0, texHeight)]
		samplerInfo.unnormalizedCoordinates = VK_FALSE; 

		// Those are mainly used for percentage-closer filtering on shadow maps.
		// https://developer.nvidia.com/gpugems/gpugems/part-ii-lighting-and-shadows/chapter-11-shadow-map-antialiasing
		samplerInfo.compareEnable = VK_FALSE;		  
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS; 

		// Mipmapping settings
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR; 
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;
		
		if (vkCreateSampler(m_device.getDevice(), &samplerInfo, nullptr, &m_textureSampler) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture sampler!");
		}
	}
}