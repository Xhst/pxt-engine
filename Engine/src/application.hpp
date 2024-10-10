#pragma once

#include "core/memory.hpp"
#include "core/events/event.hpp"
#include "graphics/window.hpp"
#include "graphics/device.hpp"
#include "graphics/renderer.hpp"
#include "scene/scene.hpp"

#include <iostream>
#include <stdexcept>

int main();

namespace CGEngine {

    class Application {
    public:
        Application();

    protected:
        Scene& getScene() {
            return m_scene;
        }

        Device& getDevice() {
            return m_device;
        }

        Window& getWindow() {
            return m_window;
        }
    
    private:
        void run();
        void onEvent(Event& event);
        bool isRunning();

        bool m_running = true;

        Window m_window{WindowData()};
        Device m_device{m_window};
        Renderer m_renderer{m_window, m_device};

        Scene m_scene{};

        friend int ::main();
    };

    Application* initApplication();
}