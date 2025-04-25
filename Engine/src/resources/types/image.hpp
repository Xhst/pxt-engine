#pragma once

#include "core/buffer.hpp"
#include "resources/resource.hpp"

namespace PXTEngine {

    class Image : public Resource {
    public:
        struct Info {
            uint32_t width = 0;
            uint32_t height = 0;
            uint16_t channels = 0;

            Info() = default;
            Info(const uint32_t width, const uint32_t height, const uint16_t channels)
                : width(width), height(height), channels(channels) {}
        };

        Image(const ResourceId& id, const Image::Info& info, const Buffer& pixels = Buffer())
            : Resource(id, Type::Image), m_info(info), m_pixels(pixels) {}

        virtual ~Image() {
            m_pixels.release();
        }

        static Type getStaticType() { return Type::Image; }

        uint32_t getWidth() const { return m_info.width; }
        uint32_t getHeight() const { return m_info.height; }
        uint16_t getChannels() const { return m_info.channels; }
        const Buffer& getPixels() const { return m_pixels; }

    protected:
        Info m_info;
        Buffer m_pixels;
    };
}