#include "graphics/render_systems/material_render_system.hpp"

#include "core/memory.hpp"
#include "core/error_handling.hpp"
#include "core/constants.hpp"
#include "graphics/resources/texture2d.hpp"
#include "graphics/resources/vk_mesh.hpp"
#include "scene/ecs/entity.hpp"

#include <stdexcept>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace PXTEngine {

    struct MaterialPushConstantData {
        glm::mat4 modelMatrix{1.f};
        glm::mat4 normalMatrix{1.f};
        glm::vec4 color{1.f};
        float specularIntensity = 0.0f;
        float shininess = 1.0f;
        int textureIndex = 0;
        int normalMapIndex = 1;
        int ambientOcclusionMapIndex = 0;
		float tilingFactor = 1.0f;
    };

    MaterialRenderSystem::MaterialRenderSystem(Context& context, Shared<DescriptorAllocatorGrowable> descriptorAllocator,
    	TextureRegistry& textureRegistry, VkRenderPass renderPass, DescriptorSetLayout& globalSetLayout,
    	VkDescriptorImageInfo shadowMapImageInfo)
	: m_context(context), m_descriptorAllocator(descriptorAllocator), m_textureRegistry(textureRegistry) {
		createDescriptorSets(shadowMapImageInfo);
        createPipelineLayout(globalSetLayout);
        createPipeline(renderPass);
    }

    MaterialRenderSystem::~MaterialRenderSystem() {
        vkDestroyPipelineLayout(m_context.getDevice(), m_pipelineLayout, nullptr);
    }

    void MaterialRenderSystem::createDescriptorSets(VkDescriptorImageInfo shadowMapImageInfo) {
		// TEXTURE DESCRIPTOR SET
        m_textureDescriptorSetLayout = DescriptorSetLayout::Builder(m_context)
			.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, m_textureRegistry.getTextureCount())
			.build();

		std::vector<VkDescriptorImageInfo> imageInfos;
		for (const auto& image : m_textureRegistry.getTextures()) {
            const auto texture = std::static_pointer_cast<Texture2D>(image);

			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = texture->getImageView();
			imageInfo.sampler = texture->getImageSampler();
			imageInfos.push_back(imageInfo);
		}

		m_descriptorAllocator->allocate(m_textureDescriptorSetLayout->getDescriptorSetLayout(), m_textureDescriptorSet);

		DescriptorWriter(m_context, *m_textureDescriptorSetLayout)
			.writeImages(0, imageInfos.data(), static_cast<uint32_t>(imageInfos.size()))
			.updateSet(m_textureDescriptorSet);

        // SHADOW MAP DESCRIPTOR SET
		m_shadowMapDescriptorSetLayout = DescriptorSetLayout::Builder(m_context)
			.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.build();

		m_descriptorAllocator->allocate(m_shadowMapDescriptorSetLayout->getDescriptorSetLayout(), m_shadowMapDescriptorSet);

		DescriptorWriter(m_context, *m_shadowMapDescriptorSetLayout)
			.writeImage(0, &shadowMapImageInfo)
			.updateSet(m_shadowMapDescriptorSet);
    }

    void MaterialRenderSystem::createPipelineLayout(DescriptorSetLayout& globalSetLayout) {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(MaterialPushConstantData);

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{
            globalSetLayout.getDescriptorSetLayout(),
            m_textureDescriptorSetLayout->getDescriptorSetLayout(),
            m_shadowMapDescriptorSetLayout->getDescriptorSetLayout()
        };

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

    void MaterialRenderSystem::createPipeline(VkRenderPass renderPass) {
        PXT_ASSERT(m_pipelineLayout != nullptr, "Cannot create pipeline before pipelineLayout");

        PipelineConfigInfo pipelineConfig{};
        Pipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = m_pipelineLayout;

		const std::vector<std::pair<VkShaderStageFlagBits, std::string>>& shaderFilePaths = {
			{VK_SHADER_STAGE_VERTEX_BIT, SPV_SHADERS_PATH + "material_shader.vert.spv"},
			{VK_SHADER_STAGE_FRAGMENT_BIT, SPV_SHADERS_PATH + "material_shader.frag.spv"}
		};

        m_pipeline = createUnique<Pipeline>(
            m_context,
            shaderFilePaths,
            pipelineConfig
        );
    }

    void MaterialRenderSystem::render(FrameInfo& frameInfo) {
        m_pipeline->bind(frameInfo.commandBuffer);

        std::array<VkDescriptorSet, 3> descriptorSets = { frameInfo.globalDescriptorSet, m_textureDescriptorSet, m_shadowMapDescriptorSet };

        vkCmdBindDescriptorSets(
            frameInfo.commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_pipelineLayout,
            0,
            descriptorSets.size(),
            descriptorSets.data(),
            0,
            nullptr
        );

        auto view = frameInfo.scene.getEntitiesWith<TransformComponent, ModelComponent>();
        for (auto entity : view) {

            const auto&[transform, model] = view.get<TransformComponent, ModelComponent>(entity);

            auto material = model.model->getMaterial();

            MaterialPushConstantData push{};
            push.modelMatrix = transform.mat4();
            push.normalMatrix = transform.normalMatrix();
            push.color = material->getAlbedoColor();
            push.specularIntensity = 1.0f;
            push.shininess = 1.0f;
            push.textureIndex = m_textureRegistry.getIndex(material->getAlbedoMap()->id);
            push.normalMapIndex = m_textureRegistry.getIndex(material->getNormalMap()->id);
            push.ambientOcclusionMapIndex = m_textureRegistry.getIndex(material->getAmbientOcclusionMap()->id);
            push.tilingFactor = 1.0f;

            vkCmdPushConstants(
                frameInfo.commandBuffer,
                m_pipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(MaterialPushConstantData),
                &push);

            auto vulkanModel = std::static_pointer_cast<VulkanMesh>(model.model);
            
            vulkanModel->bind(frameInfo.commandBuffer);
            vulkanModel->draw(frameInfo.commandBuffer);

        }
    }
}