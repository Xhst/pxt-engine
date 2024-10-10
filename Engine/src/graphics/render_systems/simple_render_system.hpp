#pragma once

#include "core/memory.hpp"
#include "graphics/camera.hpp"
#include "graphics/pipeline.hpp"
#include "graphics/swap_chain.hpp"
#include "graphics/device.hpp"
#include "scene/scene.hpp"

namespace CGEngine {

    class SimpleRenderSystem {
    public:
        SimpleRenderSystem(Device& device, VkRenderPass renderPass);
        ~SimpleRenderSystem();

        SimpleRenderSystem(const SimpleRenderSystem&) = delete;
        SimpleRenderSystem& operator=(const SimpleRenderSystem&) = delete;

        void renderScene(VkCommandBuffer commandBuffer, Scene& scene, const Camera& camera);

    private:
        void createPipelineLayout();
        void createPipeline(VkRenderPass renderPass);  
        
        Device& m_device;

        Unique<Pipeline> m_pipeline;
        VkPipelineLayout m_pipelineLayout;
    };
}