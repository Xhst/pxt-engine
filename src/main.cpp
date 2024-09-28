#include "core/base.hpp"
#include "core/window.hpp"

#include <iostream>

int main() {
    auto window = CGEngine::Window::create();

    std::cout << "Window created: " << window->getWidth() << "x" << window->getHeight() << std::endl;

    while (!window->shouldClose()) {
        glfwPollEvents();
    }

    return 0;
}