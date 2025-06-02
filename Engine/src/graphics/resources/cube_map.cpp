#include "graphics/resources/cube_map.hpp"


namespace PXTEngine {
	CubeMap::CubeMap(Context& context, const uint32_t size, const VkFormat format, const VkImageUsageFlags usageFlags)
		: VulkanImage(context, {}, Buffer()), m_imageFormat(format), m_usageFlags(usageFlags),
		  m_size(size) {
		for (int i = 0; i < 6; i++) {
			m_cubeFaceViews[i] = VK_NULL_HANDLE;
		}

		createImage();
		createImageViews();
		createSampler();
	}

	CubeMap::~CubeMap() {
		for (auto& imageView : m_cubeFaceViews) {
			vkDestroyImageView(m_context.getDevice(), imageView, nullptr);
		}
	}

	void CubeMap::createImage() {
		// Cube map image description
		VkImageCreateInfo imageCreateInfo = {};
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.format = m_imageFormat;
		imageCreateInfo.extent = { m_size, m_size, 1 };
		imageCreateInfo.mipLevels = 1;
		imageCreateInfo.arrayLayers = 6;
		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCreateInfo.usage = m_usageFlags;
		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageCreateInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

		// Create the image
		m_context.createImageWithInfo(imageCreateInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_vkImage, m_imageMemory);
	}

	void CubeMap::createImageViews() {
		// Create image view
		VkImageViewCreateInfo viewInfo = {};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
		viewInfo.format = m_imageFormat;
		viewInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G,
								VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1.0; 
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 6;
		viewInfo.image = m_vkImage;
		
		// this is the image view for the whole cube map
		m_imageView = m_context.createImageView(viewInfo);

		// now we create the image views for each face of the cube map
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.subresourceRange.layerCount = 1;
		viewInfo.image = m_vkImage;

		for (uint32_t i = 0; i < 6; i++)
		{
			viewInfo.subresourceRange.baseArrayLayer = i;
			m_cubeFaceViews[i] = m_context.createImageView(viewInfo);
		}
	}

	void CubeMap::createSampler() {
		VkSamplerCreateInfo sampler = {};
		sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		sampler.magFilter = VK_FILTER_LINEAR;
		sampler.minFilter = VK_FILTER_LINEAR;
		sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		sampler.addressModeV = sampler.addressModeU;
		sampler.addressModeW = sampler.addressModeU;
		sampler.mipLodBias = 0.0f;
		sampler.maxAnisotropy = 1.0f;
		sampler.compareOp = VK_COMPARE_OP_NEVER;
		sampler.minLod = 0.0f;
		sampler.maxLod = 1.0f;
		sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

		m_sampler = m_context.createSampler(sampler);
	}

	
}


