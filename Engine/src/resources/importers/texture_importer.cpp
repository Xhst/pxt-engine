#include "resources/importers/texture_importer.hpp"

#include "core/pch.hpp"
#include "core/buffer.hpp"
#include "graphics/resources/texture2d.hpp"

#include <stb_image.h>

namespace PXTEngine {

	Shared<Image> TextureImporter::import(ResourceManager& rm, const std::filesystem::path& filePath,
		ResourceInfo* resourceInfo) {

		int width, height, channels;

        // Currently every image is loaded as RGBA
		constexpr uint16_t requestedChannels = STBI_rgb_alpha;

		Buffer pixels;

		pixels.bytes = stbi_load(
                    filePath.string().c_str(),
                    &width,
                    &height,
                    &channels,
                    requestedChannels
        );

		pixels.size = width * height * requestedChannels;

		if (!pixels) {
            pixels.release();
			throw std::runtime_error("failed to load image from file: " + filePath.string());
		}

		ImageInfo imageInfo;

        if (resourceInfo != nullptr) {
            if (const auto* info = dynamic_cast<ImageInfo*>(resourceInfo)) {
                imageInfo = *info;
                imageInfo.width = width;
                imageInfo.height = height;
                imageInfo.channels = requestedChannels;
            } else {
                throw std::runtime_error("TextureImporter - Invalid resourceInfo type: not ImageInfo");
            }
        } else {
            imageInfo = ImageInfo(width, height, requestedChannels, RGBA8_LINEAR);
        }

		return Texture2D::create(imageInfo, pixels);
	}
}