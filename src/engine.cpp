#include "engine.hpp"

#include <iostream>

namespace CGEngine {

    void Engine::run() {
        while (isRunning()) {
            glfwPollEvents();
        }
    }

    bool Engine::isRunning() {
        return !m_window.shouldClose();
    }

}