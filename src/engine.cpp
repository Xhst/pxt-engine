#include "engine.hpp"

#include <iostream>

namespace CGEngine {

    Engine::Engine() {
        m_window = Window::create();

        std::cout << "Window created: " << m_window->getWidth() << "x" << m_window->getHeight() << std::endl;
    }

    void Engine::run() {
        while (isRunning()) {
            glfwPollEvents();
        }
    }

    bool Engine::isRunning() {
        return !m_window->shouldClose();
    }

}