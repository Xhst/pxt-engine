#pragma once

#include "core/pch.hpp"

namespace PXTEngine {

    class Event;

    /**
     * @struct WindowData
     * @brief Holds metadata about the window such as dimensions, title, and event handling.
     */
    struct WindowData
	{
		std::string title;
		uint32_t width;
		uint32_t height;
        bool frameBufferResized;

        std::function<void(Event&)> eventCallback;

        /**
         * @brief Constructs WindowData with default or provided values.
         * @param title The title of the window.
         * @param width The width of the window.
         * @param height The height of the window.
         */
		WindowData(const std::string& title = "PXT Engine", uint32_t width = 1600, uint32_t height = 900)
			: title(title), width(width), height(height), frameBufferResized(false) { }
	};

    /**
     * @class Window
     * @brief Encapsulates a GLFW window and manages its lifecycle.
     * 
     * This class handles window creation, event processing, and Vulkan surface creation.
     */
    class Window {
    public:
        Window(const WindowData& props);
        ~Window();

        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;

        /**
         * @brief Gets the width of the window.
         * @return The window width in pixels.
         */
        uint32_t getWidth() const { return m_data.width; }

        /**
         * @brief Gets the height of the window.
         * @return The window height in pixels.
         */
        uint32_t getHeight() const { return m_data.height; }

        /**
         * @brief Gets the extent (width and height) of the window for Vulkan.
         * @return A VkExtent2D structure containing the window dimensions.
         */
        VkExtent2D getExtent() const { return { m_data.width, m_data.height }; }

        /**
         * @brief Sets the event callback function for handling window events.
         * @param callback The function to handle events.
         */
        void setEventCallback(const std::function<void(Event&)>& callback) {
            m_data.eventCallback = callback;
        }

        /**
         * @brief Gets the base GLFW window.
         * @return A pointer to the GLFWwindow instance.
         */
        GLFWwindow* getBaseWindow() { return m_window; }
        
        /**
         * @brief Checks if the window was resized.
         * @return True if the framebuffer was resized, false otherwise.
         */
        bool isWindowResized() { return m_data.frameBufferResized; }
        
        void resetWindowResizedFlag() { m_data.frameBufferResized = false; }

        /**
         * @brief Checks if the window should close.
         * @return True if the window is set to close, false otherwise.
         */
        bool shouldClose() { return glfwWindowShouldClose(m_window); }

        /**
         * @brief Creates a Vulkan surface for the window.
         * @param instance The Vulkan instance.
         * @param surface Pointer to the created Vulkan surface.
         */
        void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

    private:
        /**
         * @brief Registers the GLFW callbacks for window events.
         */
        void registerCallbacks();

        GLFWwindow* m_window;
        WindowData m_data;
    };
    

}