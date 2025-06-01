#pragma once

#include "graphics/context/context.hpp"
#include "graphics/pipeline.hpp"
#include "graphics/resources/vk_skybox.hpp"
#include "graphics/descriptors/descriptors.hpp"
#include "graphics/frame_info.hpp"

#include "scene/environment.hpp"

#include <array>
#include <memory>
#include <vector>

namespace PXTEngine {

    class SkyboxRenderSystem {
    public:
        SkyboxRenderSystem(
            Context& context,
            Shared<DescriptorAllocatorGrowable> descriptorAllocator,
			Shared<Environment> environment,
            DescriptorSetLayout& globalSetLayout,
            VkRenderPass renderPass
        );
        ~SkyboxRenderSystem();

        // Delete copy constructors and assignment operators
        SkyboxRenderSystem(const SkyboxRenderSystem&) = delete;
        SkyboxRenderSystem& operator=(const SkyboxRenderSystem&) = delete;

        void render(FrameInfo& frameInfo);

    private:
        void createDescriptorSets();
        void createPipelineLayout(DescriptorSetLayout& globalSetLayout);
        void createPipeline(VkRenderPass renderPass);

        Context& m_context;
        Shared<DescriptorAllocatorGrowable> m_descriptorAllocator;
        Shared<VulkanSkybox> m_skybox;

        Unique<Pipeline> m_pipeline;

        VkPipelineLayout m_pipelineLayout;
        VkDescriptorSet m_skyboxDescriptorSet;
        Unique<DescriptorSetLayout> m_skyboxDescriptorSetLayout;
    };

}