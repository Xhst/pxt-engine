#pragma once

#include "graphics/resources/vk_image.hpp"

#include <array>

namespace PXTEngine {

	namespace CubeFace {
		constexpr uint32_t RIGHT  = 0;
		constexpr uint32_t LEFT   = 1;
		constexpr uint32_t TOP    = 2;
		constexpr uint32_t BOTTOM = 3;
		constexpr uint32_t BACK   = 4;
		constexpr uint32_t FRONT  = 5;
	}

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