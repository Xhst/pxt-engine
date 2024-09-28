#include "core/base.hpp"
#include "core/window.hpp"

int main() {
    auto window = CGEngine::Window::create();

    while (!window->shouldClose()) {
        glfwPollEvents();
    }

    return 0;
}