#pragma once

#include "core/memory.hpp"
#include "graphics/window.hpp"
#include "graphics/pipeline.hpp"
#include "graphics/swap_chain.hpp"
#include "graphics/device.hpp"


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
        void createPipelineLayout();
        void createPipeline();  
        void createCommandBuffers();
        void drawFrame();

        Window m_window{WindowData()};
        Device m_device{m_window};
        SwapChain m_swapChain{m_device, m_window.getExtent()};
        Unique<Pipeline> m_pipeline;
        VkPipelineLayout m_pipelineLayout;
        std::vector<VkCommandBuffer> m_commandBuffers;
    };
}