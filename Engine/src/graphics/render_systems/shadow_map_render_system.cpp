#include "graphics/render_systems/shadow_map_render_system.hpp"

#include "graphics/resources/image.hpp"
#include "core/memory.hpp"
#include "core/constants.hpp"
#include "scene/ecs/entity.hpp"

#include <stdexcept>
#include <cassert>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace PXTEngine {

    struct ShadowMapPushConstantData {
        glm::mat4 modelMatrix{ 1.f };
        glm::mat4 normalMatrix{ 1.f };
    };

    ShadowMapRenderSystem::ShadowMapRenderSystem(Context& context, VkDescriptorSetLayout setLayout, VkFormat offscreenFormat) : m_context(context), m_offscreenFormat(offscreenFormat) {
        createRenderPass();
        createOffscreenFrameBuffer();
        createPipelineLayout(setLayout);
        createPipeline();
    }

    ShadowMapRenderSystem::~ShadowMapRenderSystem() {
        vkDestroyPipelineLayout(m_context.getDevice(), m_pipelineLayout, nullptr);
		vkDestroyFramebuffer(m_context.getDevice(), m_offscreenFb, nullptr);
		vkDestroyRenderPass(m_context.getDevice(), m_renderPass, nullptr);
    }

    void ShadowMapRenderSystem::createRenderPass() {
		VkAttachmentDescription attachmentDescription{};
		attachmentDescription.format = m_offscreenFormat;
		attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
		attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;							// Clear depth at beginning of the render pass
		attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;						// We will read from depth, so it's important to store the depth attachment results
		attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;					// We don't care about initial layout of the attachment
		attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;// Attachment will be transitioned to shader read at render pass end

		VkAttachmentReference depthReference = {};
		depthReference.attachment = 0;
		depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;			// Attachment will be used as depth/stencil during render pass

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 0;													// No color attachments
		subpass.pDepthStencilAttachment = &depthReference;									// Reference to our depth attachment

		// Use subpass dependencies for layout transitions
		std::array<VkSubpassDependency, 2> dependencies;

		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		dependencies[1].srcSubpass = 0;
		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        VkRenderPassCreateInfo renderPassCreateInfo = {};
        renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassCreateInfo.attachmentCount = 1;
		renderPassCreateInfo.pAttachments = &attachmentDescription;
		renderPassCreateInfo.subpassCount = 1;
		renderPassCreateInfo.pSubpasses = &subpass;
		renderPassCreateInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
		renderPassCreateInfo.pDependencies = dependencies.data();

        if (vkCreateRenderPass(m_context.getDevice(), &renderPassCreateInfo, nullptr, &m_renderPass) != VK_SUCCESS) {
			throw(std::runtime_error("failed to create offscreen render pass!"));
        }
    }

	void ShadowMapRenderSystem::createOffscreenFrameBuffer() {
		// For shadow mapping we only need a depth attachment
		m_shadowMap = createUnique<Image>(m_context,
			m_shadowMapSize, m_shadowMapSize,
			m_offscreenFormat, VK_IMAGE_TILING_OPTIMAL,
			// We will sample directly from the depth attachment for the shadow mapping
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_IMAGE_ASPECT_DEPTH_BIT);

		// Create sampler to sample from depth attachment
		// Used to sample in the fragment shader for shadowed rendering
        VkSamplerCreateInfo samplerInfo = {};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.maxAnisotropy = 1.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 1.0f;
		samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

		VkSampler sampler;

		if (vkCreateSampler(m_context.getDevice(), &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
			throw(std::runtime_error("failed to create shadow map sampler!"));
		}

		m_shadowMap->setImageSampler(sampler);

		// Create frame buffer
        VkImageView attachments[1] = { m_shadowMap->getImageView() };

		VkFramebufferCreateInfo fbufCreateInfo = {};
		fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fbufCreateInfo.renderPass = m_renderPass;
		fbufCreateInfo.attachmentCount = 1;
		fbufCreateInfo.pAttachments = attachments;
		fbufCreateInfo.width = m_shadowMapSize;
		fbufCreateInfo.height = m_shadowMapSize;
		fbufCreateInfo.layers = 1;

		if (vkCreateFramebuffer(m_context.getDevice(), &fbufCreateInfo, nullptr, &m_offscreenFb) != VK_SUCCESS) {
			throw(std::runtime_error("failed to create offscreen frame buffer!"));
		}

		// Create image descriptor info for shadow map
		m_shadowMapDescriptor.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
		m_shadowMapDescriptor.imageView = m_shadowMap->getImageView();
		m_shadowMapDescriptor.sampler = m_shadowMap->getImageSampler();
	}

    void ShadowMapRenderSystem::createPipelineLayout(VkDescriptorSetLayout setLayout) {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(ShadowMapPushConstantData);

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{setLayout};

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

        if (vkCreatePipelineLayout(m_context.getDevice(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }

    void ShadowMapRenderSystem::createPipeline() {
        assert(m_pipelineLayout != nullptr && "Cannot create pipeline before pipelineLayout");

        PipelineConfigInfo pipelineConfig{};
        Pipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = m_renderPass;
        pipelineConfig.pipelineLayout = m_pipelineLayout;

		const std::vector<std::pair<VkShaderStageFlagBits, std::string>>& shaderFilepaths = {
			{VK_SHADER_STAGE_VERTEX_BIT, SPV_SHADERS_PATH + "shadow_map_creation.vert.spv"}
		};

        m_pipeline = createUnique<Pipeline>(
            m_context,
			shaderFilepaths,
            pipelineConfig
        );
    }

    void ShadowMapRenderSystem::render(FrameInfo& frameInfo) {
        m_pipeline->bind(frameInfo.commandBuffer);

        vkCmdBindDescriptorSets(
            frameInfo.commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_pipelineLayout,
            0,
            1,
            &frameInfo.globalDescriptorSet,
            0,
            nullptr
        );

        auto view = frameInfo.scene.getEntitiesWith<TransformComponent, MaterialComponent, ModelComponent>();
        for (auto entity : view) {

            const auto&[transform, material, model] = view.get<TransformComponent, MaterialComponent, ModelComponent>(entity);

            ShadowMapPushConstantData push{};
            push.modelMatrix = transform.mat4();
            push.normalMatrix = transform.normalMatrix();

			vkCmdPushConstants(
				frameInfo.commandBuffer,
				m_pipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(ShadowMapPushConstantData),
				&push);
        }
    }
}