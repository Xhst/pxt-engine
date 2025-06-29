#pragma once

#include "core/pch.hpp"
#include "graphics/pipeline.hpp"
#include "graphics/swap_chain.hpp"
#include "graphics/context/context.hpp"
#include "graphics/frame_info.hpp"
#include "graphics/descriptors/descriptors.hpp"
#include "graphics/resources/texture_registry.hpp"
#include "scene/scene.hpp"

namespace PXTEngine {

    class MaterialRenderSystem {
    public:
        MaterialRenderSystem(Context& context, Shared<DescriptorAllocatorGrowable> descriptorAllocator, TextureRegistry& textureRegistry, DescriptorSetLayout& globalSetLayout, VkRenderPass renderPass, VkDescriptorImageInfo shadowMapImageInfo);
        ~MaterialRenderSystem();

        MaterialRenderSystem(const MaterialRenderSystem&) = delete;
        MaterialRenderSystem& operator=(const MaterialRenderSystem&) = delete;

        void render(FrameInfo& frameInfo);
        void reloadShaders();

    private:
        void createDescriptorSets(VkDescriptorImageInfo shadowMapImageInfo);
        void createPipelineLayout(DescriptorSetLayout& globalSetLayout);
        void createPipeline(bool useCompiledSpirvFiles = true);
        
        Context& m_context;
        TextureRegistry& m_textureRegistry;

		VkRenderPass m_renderPassHandle;
        Unique<Pipeline> m_pipeline;
        VkPipelineLayout m_pipelineLayout;

		Shared<DescriptorAllocatorGrowable> m_descriptorAllocator;

        Unique<DescriptorSetLayout> m_shadowMapDescriptorSetLayout{};
        VkDescriptorSet m_shadowMapDescriptorSet{};

        std::array<const std::string, 2> m_shaderFilePaths = {
            "material_shader.vert",
            "material_shader.frag"
        };
    };
}