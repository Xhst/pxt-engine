#include "graphics/resources/vk_skybox.hpp"

#include "application.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <stdexcept>

namespace PXTEngine {
    Unique<VulkanSkybox> VulkanSkybox::create(const std::array<std::string, 6>& paths) {
        Context& context = Application::get().getContext();

		return createUnique<VulkanSkybox>(context, paths);
    }

	VulkanSkybox::VulkanSkybox(Context& context, const std::array<std::string, 6>& paths)
		: m_context(context) {
		// Load the skybox textures from the provided faces
		loadTextures(paths);
	}
	void VulkanSkybox::loadTextures(const std::array<std::string, 6>& paths) {
        VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
		int width, height, channels;
		uint8_t* pixels[6] = { nullptr };
		
        for (int i = 0; i < 6; ++i) {
            pixels[i] = stbi_load(paths[i].c_str(), &width, &height, &channels, STBI_rgb_alpha);

            if (!pixels[i]) {
                // Cleanup previously loaded images before throwing
                for (int j = 0; j < i; ++j) {
                    if (pixels[j]) stbi_image_free(pixels[j]);
                }
                throw std::runtime_error("Failed to load skybox texture face: " + paths[i] + " - " + stbi_failure_reason());
            }

            if (i == 0) {
                m_size = static_cast<uint32_t>(width);
                if (width != height) {

                    // Cleanup
                    for (int j = 0; j <= i; ++j) if (pixels[j]) stbi_image_free(pixels[j]);

                    throw std::runtime_error("Skybox faces must be square. Face 0 (" + paths[0] + ") is " + std::to_string(width) + "x" + std::to_string(height));
                }
            } else {
                if (static_cast<uint32_t>(width) != m_size || static_cast<uint32_t>(height) != m_size) {

                    // Cleanup
                    for (int j = 0; j <= i; ++j) if (pixels[j]) stbi_image_free(pixels[j]);

                    throw std::runtime_error("Skybox faces must have consistent dimensions. Face " + std::to_string(i) + " (" + paths[i] + ") is " +
                        std::to_string(width) + "x" + std::to_string(height) + ", expected " + std::to_string(m_size) + "x" + std::to_string(m_size));
                }
            }
        }

		VkDeviceSize faceImageSizes = m_size * m_size * 4;
        VkDeviceSize totalImageSize = faceImageSizes * 6;

        m_cubeMap = createUnique<CubeMap>(
            m_context, 
            m_size, 
            format,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
        );

        VulkanBuffer stagingBuffer(
            m_context,
            totalImageSize,
            1, // instance count
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        );
        stagingBuffer.map();

        VkDeviceSize currentOffset = 0;
        for (int i = 0; i < 6; ++i) {
            stagingBuffer.writeToBuffer((void*) pixels[i], faceImageSizes, currentOffset);
            stbi_image_free(pixels[i]); // Free CPU-side image data
            pixels[i] = nullptr; // Avoid double free
            currentOffset += faceImageSizes;
        }

        VkImageSubresourceRange cubemapSubresourceRange{};
        cubemapSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        cubemapSubresourceRange.baseMipLevel = 0;
        cubemapSubresourceRange.levelCount = 1;
        cubemapSubresourceRange.baseArrayLayer = 0;
        cubemapSubresourceRange.layerCount = 6;

        m_context.transitionImageLayoutSingleTimeCmd(
            m_cubeMap->getVkImage(),
            format,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            cubemapSubresourceRange
        );

        m_context.copyBufferToImage(
            stagingBuffer.getBuffer(),
            m_cubeMap->getVkImage(),
            m_size,
            m_size,
            6
        );

        m_context.transitionImageLayoutSingleTimeCmd(
            m_cubeMap->getVkImage(),
            format,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            cubemapSubresourceRange
        );
	}

    VkDescriptorImageInfo VulkanSkybox::getDescriptorImageInfo() const {
        VkDescriptorImageInfo imageInfo{};
        imageInfo.sampler = m_cubeMap->getImageSampler();
        imageInfo.imageView = m_cubeMap->getImageView();
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        return imageInfo;
    }

}