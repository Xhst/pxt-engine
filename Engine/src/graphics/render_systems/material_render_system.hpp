#pragma once

#include "core/memory.hpp"
#include "graphics/pipeline.hpp"
#include "graphics/swap_chain.hpp"
#include "graphics/context/context.hpp"
#include "graphics/frame_info.hpp"
#include "graphics/descriptors/descriptors.hpp"
#include "graphics/resources/texture2d.hpp"
#include "scene/scene.hpp"

namespace PXTEngine {

    class MaterialRenderSystem {
    public:
        MaterialRenderSystem(Context& context, Shared<DescriptorAllocatorGrowable> descriptorAllocator, VkRenderPass renderPass, DescriptorSetLayout& globalSetLayout, VkDescriptorImageInfo shadowMapImageInfo);
        ~MaterialRenderSystem();

        MaterialRenderSystem(const MaterialRenderSystem&) = delete;
        MaterialRenderSystem& operator=(const MaterialRenderSystem&) = delete;

        void render(FrameInfo& frameInfo);

    private:
		void loadTextures();
        void createDescriptorSets(VkDescriptorImageInfo shadowMapImageInfo);
        void createPipelineLayout(DescriptorSetLayout& globalSetLayout);
        void createPipeline(VkRenderPass renderPass);  
        
        Context& m_context;

        Unique<Pipeline> m_pipeline;
        VkPipelineLayout m_pipelineLayout;

		Shared<DescriptorAllocatorGrowable> m_descriptorAllocator;

        Unique<DescriptorSetLayout> m_textureDescriptorSetLayout{};
        Unique<DescriptorSetLayout> m_shadowMapDescriptorSetLayout{};
        VkDescriptorSet m_textureDescriptorSet{};
        VkDescriptorSet m_shadowMapDescriptorSet{};

        std::vector<Unique<Texture2D>> m_textures;
    };
}