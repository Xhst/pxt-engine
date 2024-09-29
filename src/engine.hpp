#pragma once

#include "core/memory.hpp"
#include "core/window.hpp"
#include "renderer/pipeline.hpp"
#include "device.hpp"

namespace CGEngine {

    class Engine {
    public:
        void run();

    private:
        bool isRunning();

        Window m_window{WindowData()};
        Device m_device{m_window};
        Pipeline pipeline{
            m_device,
            "../shaders/simple_shader.vert.spv",
            "../shaders/simple_shader.frag.spv",
            Pipeline::defaultPipelineConfigInfo(m_window.getWidth(), m_window.getHeight())};
    };
}