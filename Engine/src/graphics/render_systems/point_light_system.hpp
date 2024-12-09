#pragma once

#include "core/memory.hpp"
#include "scene/camera.hpp"
#include "graphics/pipeline.hpp"
#include "graphics/swap_chain.hpp"
#include "graphics/device.hpp"
#include "graphics/frame_info.hpp"
#include "scene/scene.hpp"

namespace CGEngine {

    class PointLightSystem {
    public:
        PointLightSystem(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
        ~PointLightSystem();

        PointLightSystem(const PointLightSystem&) = delete;
        PointLightSystem& operator=(const PointLightSystem&) = delete;

        void update(FrameInfo& frameInfo, GlobalUbo& ubo);
        void render(FrameInfo& frameInfo);

    private:
        void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
        void createPipeline(VkRenderPass renderPass);  
        
        Device& m_device;

        Unique<Pipeline> m_pipeline;
        VkPipelineLayout m_pipelineLayout;
    };
}