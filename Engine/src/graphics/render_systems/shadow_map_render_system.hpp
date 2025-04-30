#pragma once

#include "core/memory.hpp"
#include "graphics/pipeline.hpp"
#include "graphics/renderer.hpp"
#include "graphics/swap_chain.hpp"
#include "graphics/context/context.hpp"
#include "graphics/frame_info.hpp"
#include "graphics/resources/vk_buffer.hpp"
#include "graphics/resources/shadow_cube_map.hpp"
#include "graphics/descriptors/descriptors.hpp"

namespace PXTEngine {
    class ShadowMapRenderSystem {
    public:
        ShadowMapRenderSystem(Context& context, Shared<DescriptorAllocatorGrowable> descriptorAllocator, DescriptorSetLayout& setLayout);
        ~ShadowMapRenderSystem();

        ShadowMapRenderSystem(const ShadowMapRenderSystem&) = delete;
        ShadowMapRenderSystem& operator=(const ShadowMapRenderSystem&) = delete;

		void update(FrameInfo& frameInfo, GlobalUbo& ubo);
        void render(FrameInfo& frameInfo, Renderer& renderer);

		VkRenderPass getRenderPass() const { return m_renderPass; }
		VkFramebuffer getCubeFaceFramebuffer(uint32_t face_index) const { return m_cubeFramebuffers[face_index]; }
		VkExtent2D getExtent() const { return { m_shadowMapSize, m_shadowMapSize }; }
		VkDescriptorImageInfo getShadowMapImageInfo() const { return m_shadowMapDescriptorInfo; }
        std::array<VkDescriptorImageInfo, 6> getDebugShadowMapImageInfos() const { return m_debugImageDescriptorInfos; }

    private:
        void createUniformBuffers();
		void createDescriptorSets(DescriptorSetLayout& setLayout);
        void createRenderPass();
        void createOffscreenFrameBuffers();
        void createPipelineLayout(DescriptorSetLayout& setLayout);
        void createPipeline();  

        glm::mat4 getFaceViewMatrix(uint32_t faceIndex);
        
        const uint32_t m_shadowMapSize{ 4096 };

		// Defines the depth range used for the shadow maps
        // This should be kept as small as possible for precision
		float zNear{ 0.1f };
        float zFar{ 50.0f };

        Context& m_context;

		Shared<DescriptorAllocatorGrowable> m_descriptorAllocator;

        std::array<Unique<VulkanBuffer>, SwapChain::MAX_FRAMES_IN_FLIGHT> m_lightUniformBuffers;
        std::array<VkDescriptorSet, SwapChain::MAX_FRAMES_IN_FLIGHT> m_lightDescriptorSets;

        Unique<ShadowCubeMap> m_shadowCubeMap;
		VkDescriptorImageInfo m_shadowMapDescriptorInfo{ VK_NULL_HANDLE };
		std::array<VkDescriptorImageInfo, 6> m_debugImageDescriptorInfos;

		VkRenderPass m_renderPass;
		// The framebuffer used for the offscreen render pass. They are created from the 
		// shadowCubeMap image views (see createOffscreenFrameBuffers)
        std::array<VkFramebuffer, 6> m_cubeFramebuffers;
		Unique<VulkanImage> m_depthStencilImageFb;
        VkFormat m_offscreenDepthFormat{ VK_FORMAT_UNDEFINED };
		VkFormat m_offscreenColorFormat{ VK_FORMAT_R32_SFLOAT };

        Unique<Pipeline> m_pipeline;
        VkPipelineLayout m_pipelineLayout;
    };
}