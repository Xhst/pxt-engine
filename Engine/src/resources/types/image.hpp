#pragma once

#include "core/pch.hpp"
#include "resources/resource.hpp"

namespace PXTEngine {

	/**
	 * @enum ImageFormat
	 *
	 * @brief Enum representing different image formats.
	 * This enum is used to specify the format of images in the engine.
	 */
	enum ImageFormat : uint8_t {
		RGB8_SRGB = 0,
		RGBA8_SRGB,
		RGB8_LINEAR,
		RGBA8_LINEAR,
	};

	/**
	 * @struct ImageInfo
	 *
	 * @brief Struct representing additional information about an image resource.
	 * This struct can be used to store metadata or other relevant information about the image.
	 */
	struct ImageInfo : public ResourceInfo {
		uint32_t width = 0;
		uint32_t height = 0;
		uint16_t channels = 0;
		ImageFormat format = RGBA8_SRGB;


		ImageInfo() = default;
		ImageInfo(const uint32_t width, const uint32_t height, const uint16_t channels, 
				  const ImageFormat format = RGBA8_SRGB)
			: width(width), height(height), channels(channels), format(format) {}
		ImageInfo(const ImageInfo& other) = default;
	};

	/**
	 * @class Image
	 *
	 * @brief Represents an image resource used for rendering.
	 */
	class Image : public Resource {
    public:
        virtual uint32_t getWidth() = 0;
        virtual uint32_t getHeight() = 0;
        virtual uint16_t getChannels() = 0;
		virtual ImageFormat getFormat() = 0;

        static Type getStaticType() { return Type::Image; }
    };
}