#pragma once

#include "core/memory.hpp"
#include "scene/camera.hpp"
#include "graphics/pipeline.hpp"
#include "graphics/swap_chain.hpp"
#include "graphics/device.hpp"
#include "graphics/frame_info.hpp"
#include "scene/scene.hpp"

namespace PXTEngine {

    class SimpleRenderSystem {
    public:
        SimpleRenderSystem(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
        ~SimpleRenderSystem();

        SimpleRenderSystem(const SimpleRenderSystem&) = delete;
        SimpleRenderSystem& operator=(const SimpleRenderSystem&) = delete;

        void render(FrameInfo& framInfo);

    private:
        void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
        void createPipeline(VkRenderPass renderPass);  
        
        Device& m_device;

        Unique<Pipeline> m_pipeline;
        VkPipelineLayout m_pipelineLayout;
    };
}