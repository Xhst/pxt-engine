#pragma once

#include "core/memory.hpp"
#include "graphics/pipeline.hpp"
#include "graphics/swap_chain.hpp"
#include "graphics/context/context.hpp"
#include "graphics/frame_info.hpp"
#include "graphics/descriptors/descriptors.hpp"
#include "graphics/resources/texture_registry.hpp"
#include "graphics/resources/blas_registry.hpp"
#include "scene/scene.hpp"

namespace PXTEngine {

    class RayTracingRenderSystem {
    public:
        RayTracingRenderSystem(Context& context, Shared<DescriptorAllocatorGrowable> descriptorAllocator, TextureRegistry& textureRegistry, VkRenderPass renderPass, DescriptorSetLayout& globalSetLayout, VkDescriptorImageInfo shadowMapImageInfo);
        ~RayTracingRenderSystem();

        RayTracingRenderSystem(const RayTracingRenderSystem&) = delete;
        RayTracingRenderSystem& operator=(const RayTracingRenderSystem&) = delete;

        void render(FrameInfo& frameInfo);

    private:
		void createShaderBindingTable();
		void createTLAS();
        void updateTLAS() = 0; // not used yet
        void createPipelineLayout(DescriptorSetLayout& setLayout);
        void createPipeline(VkRenderPass renderPass);

        Context& m_context;
        TextureRegistry& m_textureRegistry;
		BLASRegistry& m_blasRegistry;

        Unique<Pipeline> m_pipeline;
        VkPipelineLayout m_pipelineLayout;

        Shared<DescriptorAllocatorGrowable> m_descriptorAllocator;
    };
}