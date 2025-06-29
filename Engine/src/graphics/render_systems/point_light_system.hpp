#pragma once

#include "core/pch.hpp"
#include "scene/camera.hpp"
#include "graphics/pipeline.hpp"
#include "graphics/swap_chain.hpp"
#include "graphics/context/context.hpp"
#include "graphics/frame_info.hpp"
#include "scene/scene.hpp"

namespace PXTEngine {

    class PointLightSystem {
    public:
        PointLightSystem(Context& context, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
        ~PointLightSystem();

        PointLightSystem(const PointLightSystem&) = delete;
        PointLightSystem& operator=(const PointLightSystem&) = delete;

        void update(FrameInfo& frameInfo, GlobalUbo& ubo);
        void render(FrameInfo& frameInfo);

    private:
        void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
        void createPipeline(VkRenderPass renderPass, bool useCompiledSpirvFiles = true);  
        
        Context& m_context;

        Unique<Pipeline> m_pipeline;
        VkPipelineLayout m_pipelineLayout;

        std::array<const std::string, 2> m_shaderFilePaths = {
            "point_light_billboard.vert",
            "point_light_billboard.frag"
        };
    };
}