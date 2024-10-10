#pragma once

#include "core/memory.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>
#include <functional>

namespace CGEngine {

    class Event;

    struct WindowData
	{
		std::string title;
		uint32_t width;
		uint32_t height;
        bool frameBufferResized;

        std::function<void(Event&)> eventCallback;

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
        void setEventCallback(const std::function<void(Event&)>& callback) {
            m_data.eventCallback = callback;
        }
        GLFWwindow* getBaseWindow() { return m_window; }
        
        bool isWindowResized() { return m_data.frameBufferResized; }
        void resetWindowResizedFlag() { m_data.frameBufferResized = false; }

        bool shouldClose() { return glfwWindowShouldClose(m_window); }

        void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

    private:
        GLFWwindow* m_window;
        WindowData m_data;

        void registerCallbacks();
    };
    

}