#include "graphics/render_systems/material_render_system.hpp"
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

    struct SimplePushConstantData {
        glm::mat4 modelMatrix{1.f};
        glm::mat4 normalMatrix{1.f};
        glm::vec4 color{1.f};
        float specularIntensity = 0.0f;
        float shininess = 1.0f;
        int textureIndex = -1;
    };

    MaterialRenderSystem::MaterialRenderSystem(Context& context, Shared<DescriptorAllocatorGrowable> descriptorAllocator, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, VkDescriptorImageInfo shadowMapImageInfo) : m_context(context), m_descriptorAllocator(descriptorAllocator) {
		loadTextures();
		createDescriptorSets(globalSetLayout, shadowMapImageInfo);
        createPipelineLayout(globalSetLayout);
        createPipeline(renderPass);
    }

    MaterialRenderSystem::~MaterialRenderSystem() {
        vkDestroyPipelineLayout(m_context.getDevice(), m_pipelineLayout, nullptr);
    }

	void MaterialRenderSystem::loadTextures() {
		std::vector<std::string> textures_name = {
			"white_pixel.png",
			"shrek_420x420.png",
			"texture.jpg",
		};

		for (const auto& texture_name : textures_name) {
			m_textures.push_back(createUnique<Image>(TEXTURES_PATH + texture_name, m_context));
		}
	}

    void MaterialRenderSystem::createDescriptorSets(VkDescriptorSetLayout globalSetLayout, VkDescriptorImageInfo shadowMapImageInfo) {
		// TEXTURE DESCRIPTOR SET
        m_textureDescriptorSetLayout = DescriptorSetLayout::Builder(m_context)
			.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, m_textures.size())
			.build();

		std::vector<VkDescriptorImageInfo> imageInfos;
		for (const auto& texture : m_textures) {
			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = texture->getImageView();
			imageInfo.sampler = texture->getImageSampler();
			imageInfos.push_back(imageInfo);
		}

		m_descriptorAllocator->allocate(m_textureDescriptorSetLayout->getDescriptorSetLayout(), m_textureDescriptorSet);

		DescriptorWriter(m_context, *m_textureDescriptorSetLayout)
			.writeImages(0, imageInfos.data(), imageInfos.size())
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

    void MaterialRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(SimplePushConstantData);

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{
            globalSetLayout,
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
        assert(m_pipelineLayout != nullptr && "Cannot create pipeline before pipelineLayout");

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

        auto view = frameInfo.scene.getEntitiesWith<TransformComponent, MaterialComponent, ModelComponent>();
        for (auto entity : view) {

            const auto&[transform, material, model] = view.get<TransformComponent, MaterialComponent, ModelComponent>(entity);

            SimplePushConstantData push{};
            push.modelMatrix = transform.mat4();
            push.normalMatrix = transform.normalMatrix();
            push.color = material.color;
            push.specularIntensity = material.specularIntensity;
            push.shininess = material.shininess;
            push.textureIndex = material.textureIndex;

            vkCmdPushConstants(
                frameInfo.commandBuffer,
                m_pipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(SimplePushConstantData),
                &push);

            auto modelPtr = model.model;
            
            modelPtr->bind(frameInfo.commandBuffer);
            modelPtr->draw(frameInfo.commandBuffer);

        }
    }
}