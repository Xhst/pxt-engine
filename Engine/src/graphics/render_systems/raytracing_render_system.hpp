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
        RayTracingRenderSystem(Context& context, Shared<DescriptorAllocatorGrowable> descriptorAllocator, TextureRegistry& textureRegistry, BLASRegistry& blasRegistry, DescriptorSetLayout& globalSetLayout);
        ~RayTracingRenderSystem();

        RayTracingRenderSystem(const RayTracingRenderSystem&) = delete;
        RayTracingRenderSystem& operator=(const RayTracingRenderSystem&) = delete;

        void update(FrameInfo& frameInfo);
        void render(FrameInfo& frameInfo);

    private:
		void createDescriptorSets();
		void defineShaderGroups();
        void createPipelineLayout(DescriptorSetLayout& setLayout);
        void createPipeline();
		void createShaderBindingTable();

        Context& m_context;
        TextureRegistry& m_textureRegistry;
		BLASRegistry& m_blasRegistry; // used only to initialize tlasBuildSystem
        
        Shared<DescriptorAllocatorGrowable> m_descriptorAllocator = nullptr;
        
        TLASBuildSystem m_tlasBuildSystem{m_context, m_blasRegistry, m_descriptorAllocator};

        Unique<Pipeline> m_pipeline;
        VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;

        std::vector<ShaderGroupInfo> m_shaderGroups{};
        Unique<VulkanBuffer> m_sbtBuffer = nullptr;
        VkStridedDeviceAddressRegionKHR m_raygenRegion;
        VkStridedDeviceAddressRegionKHR m_missRegion;
        VkStridedDeviceAddressRegionKHR m_hitRegion;
        VkStridedDeviceAddressRegionKHR m_callableRegion; // empty for now
    };
}