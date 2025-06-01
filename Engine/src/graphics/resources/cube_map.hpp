#pragma once

#include "graphics/resources/vk_image.hpp"

#include <array>

namespace PXTEngine {

	class CubeMap : public VulkanImage {
	public:
		CubeMap(Context& context, 
				uint32_t size, 
				VkFormat format,
				VkImageUsageFlags usageFlags);

		~CubeMap() override;

		VkImageView getFaceImageView(uint32_t faceIndex) const { return m_cubeFaceViews[faceIndex]; }

	private:
		uint32_t m_size; // Size of the cube map faces

		void createImage();
		void createImageViews();
		void createSampler();

		VkFormat m_imageFormat;
		VkImageUsageFlags m_usageFlags;

		std::array<VkImageView, 6> m_cubeFaceViews;
	};
}