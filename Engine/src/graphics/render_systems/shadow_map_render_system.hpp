#pragma once

#include "core/memory.hpp"
#include "graphics/pipeline.hpp"
#include "graphics/swap_chain.hpp"
#include "graphics/context/context.hpp"
#include "graphics/frame_info.hpp"
#include "graphics/resources/image.hpp"
#include "graphics/descriptors/descriptors.hpp"
#include "scene/scene.hpp"

namespace PXTEngine {

    class ShadowMapRenderSystem {
    public:
        ShadowMapRenderSystem(Context& context, Shared<DescriptorAllocatorGrowable> descriptorAllocator, DescriptorSetLayout& setLayout, VkFormat offscreenFormat);
        ~ShadowMapRenderSystem();

        ShadowMapRenderSystem(const ShadowMapRenderSystem&) = delete;
        ShadowMapRenderSystem& operator=(const ShadowMapRenderSystem&) = delete;

		void update(FrameInfo& frameInfo, GlobalUbo& ubo);
        void render(FrameInfo& frameInfo);

		VkRenderPass getRenderPass() const { return m_renderPass; }
		VkFramebuffer getOffscreenFramebuffer() const { return m_offscreenFb; }
		VkExtent2D getExtent() const { return { m_shadowMapSize, m_shadowMapSize }; }
		VkDescriptorImageInfo getShadowMapImageInfo() const { return m_shadowMapDescriptor; }

    private:
        void createUniformBuffers();
		void createDescriptorSets(DescriptorSetLayout& setLayout);
        void createRenderPass();
        void createOffscreenFrameBuffers();
        void createPipelineLayout(DescriptorSetLayout& setLayout);
        void createPipeline();  
        
        const uint32_t m_shadowMapSize{ 1024 };

		// Defines the depth range used for the shadow maps
        // This should be kept as small as possible for precision
		float zNear{ 0.1f };
        float zFar{ 102 };

        Context& m_context;

		Shared<DescriptorAllocatorGrowable> m_descriptorAllocator;

        std::array<Unique<Buffer>, SwapChain::MAX_FRAMES_IN_FLIGHT> m_lightUniformBuffers;
        std::array<VkDescriptorSet, SwapChain::MAX_FRAMES_IN_FLIGHT> m_lightDescriptorSets;

        Unique<Image> m_shadowCubeMap;
		VkDescriptorImageInfo m_shadowMapDescriptor{ VK_NULL_HANDLE };

		VkRenderPass m_renderPass;
		// The framebuffer used for the offscreen render pass. They are created from the 
		// shadowCubeMap image views (see createOffscreenFrameBuffers)
        std::array<VkFramebuffer, 6> m_cubeFramebuffers;
        VkFormat m_offscreenDepthFormat;
		VkFormat m_offscreenColorFormat{ VK_FORMAT_R32_SFLOAT };

        Unique<Pipeline> m_pipeline;
        VkPipelineLayout m_pipelineLayout;
    };
}