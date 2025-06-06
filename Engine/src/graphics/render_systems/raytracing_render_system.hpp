#pragma once

#include "core/memory.hpp"
#include "graphics/pipeline.hpp"
#include "graphics/swap_chain.hpp"
#include "graphics/frame_info.hpp"
#include "graphics/descriptors/descriptors.hpp"
#include "graphics/resources/texture_registry.hpp"
#include "graphics/resources/material_registry.hpp"
#include "graphics/resources/vk_skybox.hpp"
#include "graphics/render_systems/raytracing_scene_manager_system.hpp"
#include "graphics/renderer.hpp"
#include "scene/scene.hpp"
#include "scene/environment.hpp"

namespace PXTEngine {

    class RayTracingRenderSystem {
    public:
        RayTracingRenderSystem(Context& context, Shared<DescriptorAllocatorGrowable> descriptorAllocator, TextureRegistry& textureRegistry, MaterialRegistry& materialRegistry, BLASRegistry& blasRegistry, Shared<Environment> environment, DescriptorSetLayout& globalSetLayout, Shared<VulkanImage> sceneImage);
        ~RayTracingRenderSystem();

        RayTracingRenderSystem(const RayTracingRenderSystem&) = delete;
        RayTracingRenderSystem& operator=(const RayTracingRenderSystem&) = delete;

        void update(FrameInfo& frameInfo);
        void render(FrameInfo& frameInfo, Renderer& renderer);
		void transitionImageToShaderReadOnlyOptimal(FrameInfo& frameInfo);

        void updateSceneImage(Shared<VulkanImage> sceneImage);

    private:
		void createDescriptorSets();
		void defineShaderGroups();
        void createPipelineLayout(DescriptorSetLayout& setLayout);
        void createPipeline();
		void createShaderBindingTable();

        Context& m_context;
        TextureRegistry& m_textureRegistry;
		MaterialRegistry& m_materialRegistry;
		BLASRegistry& m_blasRegistry; // used only to initialize tlasBuildSystem
		Shared<Environment> m_environment = nullptr;
		Shared<VulkanSkybox> m_skybox = nullptr;
        
        Shared<DescriptorAllocatorGrowable> m_descriptorAllocator = nullptr;
        
        RayTracingSceneManagerSystem m_rtSceneManager{m_context, m_materialRegistry, m_blasRegistry, m_descriptorAllocator};

        Unique<Pipeline> m_pipeline;
        VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;

        std::vector<ShaderGroupInfo> m_shaderGroups{};
        Unique<VulkanBuffer> m_sbtBuffer = nullptr;
        VkStridedDeviceAddressRegionKHR m_raygenRegion;
        VkStridedDeviceAddressRegionKHR m_missRegion;
        VkStridedDeviceAddressRegionKHR m_hitRegion;
        VkStridedDeviceAddressRegionKHR m_callableRegion; // empty for now

        Shared<VulkanImage> m_sceneImage = nullptr;
		VkDescriptorSet m_storageImageDescriptorSet = VK_NULL_HANDLE;
		Unique<DescriptorSetLayout> m_storageImageDescriptorSetLayout = nullptr;
    };
}