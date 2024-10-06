#pragma once

#include "core/memory.hpp"
#include "graphics/window.hpp"
#include "graphics/device.hpp"
#include "graphics/renderer.hpp"
#include "scene/scene.hpp"

namespace CGEngine {

    class Engine {
    public:
        Engine();
        ~Engine();

        Engine(const Engine&) = delete;
        Engine& operator=(const Engine&) = delete;
        
        void run();

    private:
        bool isRunning();
        void loadScene();

        Window m_window{WindowData()};
        Device m_device{m_window};
        Renderer m_renderer{m_window, m_device};

        Scene m_scene{};
    };
}