#pragma once

#include "core/memory.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

namespace CGEngine {

    struct WindowData
	{
		std::string title;
		uint32_t width;
		uint32_t height;

		WindowData(const std::string& title = "CG Engine", uint32_t width = 1600, uint32_t height = 900)
			: title(title), width(width), height(height) { }
	};

    class Window {
    public:
        Window(const WindowData& props);
        ~Window();

        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;

        uint32_t getWidth() const { return m_data.width; }
        uint32_t getHeight() const { return m_data.height; }
        VkExtent2D getExtent() const { return { m_data.width, m_data.height }; }

        bool shouldClose() { return glfwWindowShouldClose(m_window); }

        void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

    private:
        GLFWwindow* m_window;
        WindowData m_data;
    };
    

}