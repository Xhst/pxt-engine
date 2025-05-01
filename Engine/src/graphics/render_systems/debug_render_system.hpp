#pragma once

#include "core/memory.hpp"
#include "graphics/pipeline.hpp"
#include "graphics/swap_chain.hpp"
#include "graphics/context/context.hpp"
#include "graphics/frame_info.hpp"
#include "graphics/descriptors/descriptors.hpp"
#include "scene/scene.hpp"

namespace PXTEngine {

    class DebugRenderSystem {
    public:
        DebugRenderSystem(Context& context, Shared<DescriptorAllocatorGrowable> descriptorAllocator, VkRenderPass renderPass, DescriptorSetLayout& globalSetLayout);
        ~DebugRenderSystem();

        DebugRenderSystem(const DebugRenderSystem&) = delete;
        DebugRenderSystem& operator=(const DebugRenderSystem&) = delete;

        void render(FrameInfo& frameInfo);
        void updateUi();

    private:
        void createPipelineLayout(DescriptorSetLayout& globalSetLayout);
        void createPipelines(VkRenderPass renderPass);  
        
        Context& m_context;

        Unique<Pipeline> m_pipelineWireframe;
		Unique<Pipeline> m_pipelineSolid;
        VkPipelineLayout m_pipelineLayout;

		Shared<DescriptorAllocatorGrowable> m_descriptorAllocator;

        bool m_enableWireframe = false;
        bool m_enableNormals = false;
    };
}