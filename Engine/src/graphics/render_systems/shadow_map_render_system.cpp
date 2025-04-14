#include "graphics/render_systems/shadow_map_render_system.hpp"

#include "graphics/resources/texture2d.hpp"
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
    };

    ShadowMapRenderSystem::ShadowMapRenderSystem(Context& context, Shared<DescriptorAllocatorGrowable> descriptorAllocator, DescriptorSetLayout& setLayout, VkFormat offscreenFormat)
		: m_context(context),
		  m_offscreenFormat(offscreenFormat),
		  m_descriptorAllocator(descriptorAllocator) {
		createUniformBuffers();
		createDescriptorSets(setLayout);
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

	void ShadowMapRenderSystem::createUniformBuffers() {
		// Create uniform buffer for each frame in flight
		for (size_t i = 0; i < m_lightUniformBuffers.size(); i++) {
			m_lightUniformBuffers[i] = createUnique<Buffer>(
				m_context,
				sizeof(GlobalUbo),
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
			);
		}
	}

	void ShadowMapRenderSystem::createDescriptorSets(DescriptorSetLayout& setLayout) {
		// Create descriptor set for each frame in flight
		for (int i = 0; i < m_lightDescriptorSets.size(); i++) {
			auto bufferInfo = m_lightUniformBuffers[i]->descriptorInfo();

			m_descriptorAllocator->allocate(setLayout.getDescriptorSetLayout(), m_lightDescriptorSets[i]);

			DescriptorWriter(m_context, setLayout)
				.writeBuffer(0, &bufferInfo)
				.updateSet(m_lightDescriptorSets[i]);
		}
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
		m_shadowMap = createUnique<Texture2D>(TEXTURES_PATH + "white_pixel.png", m_context);

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

    void ShadowMapRenderSystem::createPipelineLayout(DescriptorSetLayout& setLayout) {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(ShadowMapPushConstantData);

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{setLayout.getDescriptorSetLayout()};

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

		const std::vector<std::pair<VkShaderStageFlagBits, std::string>>& shaderFilePaths = {
			{VK_SHADER_STAGE_VERTEX_BIT, SPV_SHADERS_PATH + "cube_shadow_map_creation.vert.spv"},
			{ VK_SHADER_STAGE_FRAGMENT_BIT, SPV_SHADERS_PATH + "cube_shadow_map_creation.frag.spv" }
		};

        m_pipeline = createUnique<Pipeline>(
            m_context,
			shaderFilePaths,
            pipelineConfig
        );
    }

	void ShadowMapRenderSystem::update(FrameInfo& frameInfo) {
		// to set the projection (square depth map)
		uniformDataOffscreen.projection = glm::perspective((float)(M_PI / 2.0), 1.0f, zNear, zFar);
		// to set the view matrix (will be overwritten depending on the cube face - for now is identity matrix)
		uniformDataOffscreen.view = glm::mat4(1.0f);
		// this will create a translation matrix to translate the model vertices by the light position
		uniformDataOffscreen.model = glm::translate(glm::mat4(1.0f), glm::vec3(-lightPos.x, -lightPos.y, -lightPos.z));

		memcpy(uniformBuffers.offscreen.mapped, &uniformDataOffscreen, sizeof(UniformData));
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