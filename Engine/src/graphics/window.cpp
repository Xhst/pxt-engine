#include "graphics/window.hpp"

#include "core/events/event.hpp"
#include "core/events/window_event.hpp"
#include "core/events/keyboard_event.hpp"
#include "core/events/mouse_event.hpp"
#include "core/input/mapper/glfw_input_mapper.hpp"

namespace PXTEngine {

    Window::Window(const WindowData& props): m_data(props) {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        
        m_window = glfwCreateWindow(props.width, props.height, props.title.c_str(), nullptr, nullptr);

        glfwSetWindowUserPointer(m_window, &m_data);

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
  
        glfwSetWindowCloseCallback(m_window, [](GLFWwindow* window) {
			WindowData& data = *(WindowData*) glfwGetWindowUserPointer(window);

			WindowCloseEvent event;
			data.eventCallback(event);
		});

        glfwSetFramebufferSizeCallback(m_window, [](GLFWwindow* window, int width, int height) {
            WindowData& data = *(WindowData*) glfwGetWindowUserPointer(window);
            
            data.width = static_cast<uint32_t>(width);
            data.height = static_cast<uint32_t>(height);
            data.frameBufferResized = true;

            WindowResizeEvent event(width, height);
            data.eventCallback(event);
        });

        glfwSetKeyCallback(m_window, [](GLFWwindow* window, int glfwKey, int scancode, int action, int mods) {
			WindowData& data = *(WindowData*) glfwGetWindowUserPointer(window);

            KeyCode key = mapGLFWKey(glfwKey);

			switch (action) {
				case GLFW_PRESS:
				{
					KeyPressEvent event(key);
					data.eventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					KeyReleaseEvent event(key);
					data.eventCallback(event);
					break;
				}
				case GLFW_REPEAT:
				{
					KeyPressEvent event(key, 1);
					data.eventCallback(event);
					break;
				}
			}
		});

        glfwSetCharCallback(m_window, [](GLFWwindow* window, unsigned int glfwKey)
		{
			WindowData& data = *(WindowData*) glfwGetWindowUserPointer(window);

            KeyCode key = mapGLFWKey(glfwKey);

			KeyDownEvent event(key);
			data.eventCallback(event);
		});

		glfwSetMouseButtonCallback(m_window, [](GLFWwindow* window, int glfwButton, int action, int mods)
		{
			WindowData& data = *(WindowData*) glfwGetWindowUserPointer(window);
            
			MouseButton button = mapGLFWMouseButton(glfwButton);

			switch (action)
			{
				case GLFW_PRESS:
				{
					MouseButtonPressEvent event(button);
					data.eventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					MouseButtonReleaseEvent event(button);
					data.eventCallback(event);
					break;
				}
			}
		});

		glfwSetScrollCallback(m_window, [](GLFWwindow* window, double xOffset, double yOffset)
		{
			WindowData& data = *(WindowData*) glfwGetWindowUserPointer(window);

			MouseScrollEvent event(xOffset, yOffset);
			data.eventCallback(event);
		});

		glfwSetCursorPosCallback(m_window, [](GLFWwindow* window, double xPos, double yPos)
		{
			WindowData& data = *(WindowData*) glfwGetWindowUserPointer(window);

			MouseMoveEvent event(xPos, yPos);
			data.eventCallback(event);
		});


    }
}