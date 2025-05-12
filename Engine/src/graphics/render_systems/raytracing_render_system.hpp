#pragma once

#include "core/memory.hpp"
#include "graphics/pipeline.hpp"
#include "graphics/swap_chain.hpp"
#include "graphics/frame_info.hpp"
#include "graphics/descriptors/descriptors.hpp"
#include "graphics/resources/texture_registry.hpp"
#include "graphics/render_systems/tlas_build_system.hpp"
#include "scene/scene.hpp"

namespace PXTEngine {

    class RayTracingRenderSystem {
    public:
        RayTracingRenderSystem(Context& context, Shared<DescriptorAllocatorGrowable> descriptorAllocator, TextureRegistry& textureRegistry, BLASRegistry& blasRegistry, VkRenderPass renderPass, DescriptorSetLayout& globalSetLayout);
        ~RayTracingRenderSystem();

        RayTracingRenderSystem(const RayTracingRenderSystem&) = delete;
        RayTracingRenderSystem& operator=(const RayTracingRenderSystem&) = delete;

        void update(FrameInfo& frameInfo);
        void render(FrameInfo& frameInfo);

    private:
		void createShaderBindingTable();
        void createPipelineLayout(DescriptorSetLayout& setLayout);
        void createPipeline(VkRenderPass renderPass);

        Context& m_context;
        TextureRegistry& m_textureRegistry;
		BLASRegistry& m_blasRegistry; // used only to initialize tlasBuildSystem
        TLASBuildSystem m_tlasBuildSystem{m_context, m_blasRegistry};

        Unique<Pipeline> m_pipeline;
        VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;

        Shared<DescriptorAllocatorGrowable> m_descriptorAllocator;
    };
}