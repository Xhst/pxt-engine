#include "core/window.hpp"

namespace CGEngine {

    Window::Window(const WindowData& props): m_data(props) {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        m_window = glfwCreateWindow(props.width, props.height, props.title.c_str(), nullptr, nullptr);
    }

    Window::~Window() {
        glfwDestroyWindow(m_window);
        glfwTerminate();
    }

    Unique<Window> Window::create(const WindowData& props) {
        return createUnique<Window>(props);
    }

}