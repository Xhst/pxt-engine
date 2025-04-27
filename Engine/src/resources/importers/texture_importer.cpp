#include "resources/importers/texture_importer.hpp"

#include "core/buffer.hpp"
#include "graphics/resources/texture2d.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <stdexcept>

namespace PXTEngine {

	Shared<Image> TextureImporter::import(const std::filesystem::path& filePath) {
		int width, height, channels;

        // Currently every image is loaded as RGBA
        int requestedChannels = STBI_rgb_alpha;

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

		Image::Info imageInfo{};
		imageInfo.width = width;
		imageInfo.height = height;
		imageInfo.channels = requestedChannels;

		return Texture2D::create(filePath.string(), imageInfo, pixels);
	}
}