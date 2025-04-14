#pragma once

#include "graphics/resources/image.hpp"

namespace PXTEngine {
	/**
	 * @class ShadowCubeMap
	 * @brief Represents a Vulkan Cube Map texture for shadow mapping.
	 * 
	 * This class wants to encapsulate the creation and management of a Vulkan cube map texture
	 * Every face of the cube map is a separate image view. We also have an image view for the whole cube map.
	 * The format will be VK_FORMAT_R32_SFLOAT for every face by default.
	 * 
	 */
	class ShadowCubeMap : public Image {
	public:
		ShadowCubeMap(Context& context, VkFormat format = VK_FORMAT_R32_SFLOAT, uint32_t size);
		virtual ~ShadowCubeMap();

		VkFramebuffer getFaceFramebuffer(uint32_t faceIndex) const { return m_cubeFramebuffers[faceIndex]; }
		VkImageView getFaceImageView(uint32_t faceIndex) const { return m_cubeFaceViews[faceIndex]; }

	private:
		uint32_t m_size; // Size of the cube map faces

		void createSCMImage();
		void createSCMImageViews();
		void createSCMSampler();

		std::array<VkImageView, 6> m_cubeFaceViews;
	};
}