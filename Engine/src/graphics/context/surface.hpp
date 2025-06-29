#pragma once

#include "core/pch.hpp"
#include "graphics/window.hpp"
#include "graphics/context/instance.hpp"

namespace PXTEngine {

    /**
     * @class Surface
     * @brief Represents a Vulkan surface for rendering.
     *
     * This class encapsulates the Vulkan surface and provides methods for creating and managing it.
     */
    class Surface {
    public:
        Surface(Window& window, Instance& instance);
        ~Surface();

        Surface(const Surface&) = delete;
        Surface& operator=(const Surface&) = delete;

        VkSurfaceKHR getSurface() const { return m_surface; }

    private:
        Window& m_window;
        Instance& m_instance;
        VkSurfaceKHR m_surface;
    };

}