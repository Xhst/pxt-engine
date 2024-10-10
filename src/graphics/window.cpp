#include "graphics/window.hpp"

#include <stdexcept>

namespace CGEngine {

    Window::Window(const WindowData& props): m_data(props) {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        
        m_window = glfwCreateWindow(props.width, props.height, props.title.c_str(), nullptr, nullptr);

        glfwSetWindowUserPointer(m_window, this);

        registerCallbacks();
    }

    Window::~Window() {
        glfwDestroyWindow(m_window);
        glfwTerminate();
    }

    void Window::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) {
        if (glfwCreateWindowSurface(instance, m_window, nullptr, surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
    }


    void Window::registerCallbacks() {

        // Set the framebuffer resize callback
        glfwSetFramebufferSizeCallback(m_window, [](GLFWwindow* window, int width, int height) {
            auto new_window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
            new_window->m_frameBufferResized = true;
            new_window->m_data.width = static_cast<uint32_t>(width);
            new_window->m_data.height = static_cast<uint32_t>(height);
        });


    }
}