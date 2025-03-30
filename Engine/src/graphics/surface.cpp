#include "./surface.hpp"

namespace PXTEngine {

    Surface::Surface(Window& window, Instance& instance) : m_window(window), m_instance(instance) {

        m_window.createWindowSurface(m_instance.getVkInstance(), &m_surface);
    }

    Surface::~Surface() {
        vkDestroySurfaceKHR(m_instance.getVkInstance(), m_surface, nullptr);
    }
}