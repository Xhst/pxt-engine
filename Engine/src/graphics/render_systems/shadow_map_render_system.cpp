#include "graphics/render_systems/shadow_map_render_system.hpp"

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
		// it will be modified to translate the object to the light position (i think so?)
        glm::mat4 modelMatrix{ 1.f };
		// it will be modified to render the different faces
		glm::mat4 cubeFaceView{ 1.f };
    };

    ShadowMapRenderSystem::ShadowMapRenderSystem(Context& context, Shared<DescriptorAllocatorGrowable> descriptorAllocator, DescriptorSetLayout& setLayout, VkFormat offscreenDepthFormat)
		: m_context(context),
		  m_offscreenDepthFormat(offscreenDepthFormat),
		  m_descriptorAllocator(descriptorAllocator) {
		createUniformBuffers();
		createDescriptorSets(setLayout);
		createRenderPass();
        createOffscreenFrameBuffers();
        createPipelineLayout(setLayout);
        createPipeline();
    }

    ShadowMapRenderSystem::~ShadowMapRenderSystem() {
        vkDestroyPipelineLayout(m_context.getDevice(), m_pipelineLayout, nullptr);
		vkDestroyRenderPass(m_context.getDevice(), m_renderPass, nullptr);

		for (auto& framebuffer : m_cubeFramebuffers) {
			vkDestroyFramebuffer(m_context.getDevice(), framebuffer, nullptr);
		}
    }

	void ShadowMapRenderSystem::createUniformBuffers() {
		// Create uniform buffer for each frame in flight
		for (size_t i = 0; i < m_lightUniformBuffers.size(); i++) {
			m_lightUniformBuffers[i] = createUnique<Buffer>(
				m_context,
				sizeof(GlobalUbo),
				1,
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
		// offscreen attachments
		VkAttachmentDescription osAttachments[2] = {};

		// Find a suitable depth format for
		bool isDepthFormatValid = m_context.getSupportedDepthFormat(&m_offscreenDepthFormat);
		assert(isDepthFormatValid && "No depth format available");

		// Color attachment
		osAttachments[0].format = m_offscreenColorFormat;
		osAttachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
		osAttachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		osAttachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		osAttachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		osAttachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		osAttachments[0].initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		osAttachments[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		// Depth attachment
		osAttachments[1].format = m_offscreenDepthFormat;
		osAttachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
		osAttachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		osAttachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		osAttachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		osAttachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		osAttachments[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		osAttachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorReference = {};
		colorReference.attachment = 0;
		colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthReference = {};
		depthReference.attachment = 1;
		depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorReference;
		subpass.pDepthStencilAttachment = &depthReference;

		VkRenderPassCreateInfo renderPassCreateInfo = {};
		renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassCreateInfo.attachmentCount = 2;
		renderPassCreateInfo.pAttachments = osAttachments;
		renderPassCreateInfo.subpassCount = 1;
		renderPassCreateInfo.pSubpasses = &subpass;

		if (vkCreateRenderPass(m_context.getDevice(), &renderPassCreateInfo, nullptr, &m_renderPass) != VK_SUCCESS) {
			throw std::runtime_error("failed to create offscreen render pass!");
		}
    }

	void ShadowMapRenderSystem::createOffscreenFrameBuffers() {
		// For shadow mapping here we need 6 framebuffers, one for each face of the cube map
		// The class will handle this for us. It will create image views for each face, which
		// we can use to then create the framebuffers for this class
		m_shadowCubeMap = createUnique<ShadowCubeMap>(m_context, m_offscreenColorFormat, m_shadowMapSize);

		// ------------- Create framebuffers for each face of the cube map -------------

		// The color attachment is the cube map image view (we have 6, one for each framebuffer).
		// While the depth stencil is the same for all framebuffers. We will create the latter now
		// and then copy the cube face image views to the framebuffer color attachments

		// Depth stencil attachment
		VkImageCreateInfo imageCreateInfo = {};
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.format = m_offscreenDepthFormat;
		imageCreateInfo.extent = { m_shadowMapSize, m_shadowMapSize, 1 };
		imageCreateInfo.mipLevels = 1;
		imageCreateInfo.arrayLayers = 1;
		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		// Image of the framebuffer is blit source
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		Unique<Image> depthStencilImage = createUnique<Image>(m_context, imageCreateInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		VkImageSubresourceRange subresourceRange = {};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = 1;
		subresourceRange.layerCount = 1;

		// TODO: verify source and destination access masks
		m_context.transitionImageLayout(
			depthStencilImage->getVkImage(), 
			depthStencilImage->getImageFormat(),
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
			subresourceRange);

		VkImageViewCreateInfo depthStencilViewInfo = {};
		depthStencilViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		depthStencilViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		depthStencilViewInfo.format = m_offscreenDepthFormat;
		depthStencilViewInfo.flags = 0;
		depthStencilViewInfo.subresourceRange = {};
		depthStencilViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		if (m_offscreenDepthFormat >= VK_FORMAT_D16_UNORM_S8_UINT) {
			depthStencilViewInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
		depthStencilViewInfo.subresourceRange.baseMipLevel = 0;
		depthStencilViewInfo.subresourceRange.levelCount = 1;
		depthStencilViewInfo.subresourceRange.baseArrayLayer = 0;
		depthStencilViewInfo.subresourceRange.layerCount = 1;

		depthStencilImage->createImageView(depthStencilViewInfo);

		// Create framebuffers for each face of the cube map
		VkImageView attachments[2];
		attachments[1] = depthStencilImage->getImageView();

		VkFramebufferCreateInfo fbufCreateInfo = {};
		fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fbufCreateInfo.renderPass = m_renderPass;
		fbufCreateInfo.attachmentCount = 2;
		fbufCreateInfo.pAttachments = attachments;
		fbufCreateInfo.width = m_shadowMapSize;
		fbufCreateInfo.height = m_shadowMapSize;
		fbufCreateInfo.layers = 1;

		for (uint32_t i = 0; i < 6; i++)
		{
			attachments[0] = m_shadowCubeMap->getFaceImageView(i);
			if (vkCreateFramebuffer(m_context.getDevice(), &fbufCreateInfo, nullptr, &m_cubeFramebuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create shadow map render system framebuffer (face " + std::to_string(i) + ")!");
			}
		}

		// -----------------------------------------------------------------------------

		// Create image descriptor info for shadow map
		m_shadowMapDescriptor.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
		m_shadowMapDescriptor.imageView = m_shadowCubeMap->getImageView();
		m_shadowMapDescriptor.sampler = m_shadowCubeMap->getImageSampler();
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

	void ShadowMapRenderSystem::update(FrameInfo& frameInfo, GlobalUbo& ubo) {
		#if 0
// to set the projection (square depth map)
		uniformDataOffscreen.projection = glm::perspective((float)(M_PI / 2.0), 1.0f, zNear, zFar);
		// to set the view matrix (will be overwritten depending on the cube face - for now is identity matrix)
		uniformDataOffscreen.view = glm::mat4(1.0f);
		// this will create a translation matrix to translate the model vertices by the light position
		uniformDataOffscreen.model = glm::translate(glm::mat4(1.0f), glm::vec3(-lightPos.x, -lightPos.y, -lightPos.z));

		memcpy(uniformBuffers.offscreen.mapped, &uniformDataOffscreen, sizeof(UniformData));

#endif
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