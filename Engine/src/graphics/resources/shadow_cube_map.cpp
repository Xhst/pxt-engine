#include "graphics/resources/shadow_cube_map.hpp"


namespace PXTEngine {
	ShadowCubeMap::ShadowCubeMap(Context& context, const uint32_t size, const VkFormat format)
		: VulkanImage(context, {}, Buffer()), m_imageFormat(format),
		  m_size(size) {
		for (int i = 0; i < 6; i++) {
			m_cubeFaceViews[i] = VK_NULL_HANDLE;
		}

		createSCMImage();
		transitionSCMLayout();
		createSCMImageViews();
		createSCMSampler();
	}

	ShadowCubeMap::~ShadowCubeMap() {
		for (auto& imageView : m_cubeFaceViews) {
			vkDestroyImageView(m_context.getDevice(), imageView, nullptr);
		}
	}

	void ShadowCubeMap::createSCMImage() {
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
		imageCreateInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageCreateInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

		// Create the image
		m_context.createImageWithInfo(imageCreateInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_vkImage, m_imageMemory);
	}

	void ShadowCubeMap::transitionSCMLayout() {
		VkImageSubresourceRange subresourceRange = {};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = 1;
		subresourceRange.layerCount = 6;

		m_context.transitionImageLayout(
			m_vkImage,
			m_imageFormat,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			subresourceRange);
	}

	void ShadowCubeMap::createSCMImageViews() {
		// Create image view
		VkImageViewCreateInfo viewInfo = {};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
		viewInfo.format = m_imageFormat;
		viewInfo.components = { VK_COMPONENT_SWIZZLE_R };
		viewInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
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

	void ShadowCubeMap::createSCMSampler() {
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


