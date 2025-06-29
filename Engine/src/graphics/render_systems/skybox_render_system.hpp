#pragma once

#include "core/pch.hpp"
#include "graphics/context/context.hpp"
#include "graphics/pipeline.hpp"
#include "graphics/resources/vk_skybox.hpp"
#include "graphics/descriptors/descriptors.hpp"
#include "graphics/frame_info.hpp"

#include "scene/environment.hpp"

namespace PXTEngine {

    class SkyboxRenderSystem {
    public:
        SkyboxRenderSystem(
            Context& context,
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
        void createPipelineLayout(DescriptorSetLayout& globalSetLayout);
        void createPipeline(VkRenderPass renderPass, bool useCompiledSpirvFiles = true);

        Context& m_context;
        Shared<VulkanSkybox> m_skybox;

        Unique<Pipeline> m_pipeline;
        VkPipelineLayout m_pipelineLayout;

        std::array<const std::string, 2> m_shaderFilePaths = {
            "skybox.vert",
            "skybox.frag"
        };
    };

}