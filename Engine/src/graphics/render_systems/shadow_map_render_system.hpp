#pragma once

#include "core/memory.hpp"
#include "graphics/pipeline.hpp"
#include "graphics/swap_chain.hpp"
#include "graphics/context/context.hpp"
#include "graphics/frame_info.hpp"
#include "graphics/resources/image.hpp"
#include "scene/scene.hpp"

namespace PXTEngine {

    class ShadowMapRenderSystem {
    public:
        ShadowMapRenderSystem(Context& context, VkDescriptorSetLayout setLayout, VkFormat offscreenFormat);
        ~ShadowMapRenderSystem();

        ShadowMapRenderSystem(const ShadowMapRenderSystem&) = delete;
        ShadowMapRenderSystem& operator=(const ShadowMapRenderSystem&) = delete;

        void render(FrameInfo& frameInfo);

		VkRenderPass getRenderPass() const { return m_renderPass; }
		VkFramebuffer getOffscreenFramebuffer() const { return m_offscreenFb; }
		VkExtent2D getExtent() const { return { m_shadowMapSize, m_shadowMapSize }; }
		VkDescriptorImageInfo getShadowMapImageInfo() const { return m_shadowMapDescriptor; }

    private:
        void createRenderPass();
        void createOffscreenFrameBuffer();
        void createPipelineLayout(VkDescriptorSetLayout setLayout);
        void createPipeline();  
        
        const uint32_t m_shadowMapSize{ 2048 };

        Context& m_context;

        Unique<Image> m_shadowMap;
		VkDescriptorImageInfo m_shadowMapDescriptor{};

		VkRenderPass m_renderPass;
        VkFramebuffer m_offscreenFb;
        VkFormat m_offscreenFormat;

        Unique<Pipeline> m_pipeline;
        VkPipelineLayout m_pipelineLayout;
    };
}