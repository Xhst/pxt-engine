#include "graphics/render_systems/material_render_system.hpp"

#include "core/memory.hpp"
#include "core/diagnostics.hpp"
#include "core/constants.hpp"
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
        int metallicMapIndex = 0;
		int roughnessMapIndex = 0;
		float tilingFactor = 1.0f;
    };

    MaterialRenderSystem::MaterialRenderSystem(Context& context, Shared<DescriptorAllocatorGrowable> descriptorAllocator,
    	TextureRegistry& textureRegistry, DescriptorSetLayout& globalSetLayout,
    	VkRenderPass renderPass, VkDescriptorImageInfo shadowMapImageInfo)
        : m_context(context),
        m_descriptorAllocator(descriptorAllocator),
        m_textureRegistry(textureRegistry)
    {
		createDescriptorSets(shadowMapImageInfo);
        createPipelineLayout(globalSetLayout);
        createPipeline(renderPass);
    }

    MaterialRenderSystem::~MaterialRenderSystem() {
        vkDestroyPipelineLayout(m_context.getDevice(), m_pipelineLayout, nullptr);
    }

    void MaterialRenderSystem::createDescriptorSets(VkDescriptorImageInfo shadowMapImageInfo) {
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
            m_textureRegistry.getDescriptorSetLayout(),
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

        RasterizationPipelineConfigInfo pipelineConfig{};
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

        std::array<VkDescriptorSet, 3> descriptorSets = { frameInfo.globalDescriptorSet, m_textureRegistry.getDescriptorSet(), m_shadowMapDescriptorSet};

        vkCmdBindDescriptorSets(
            frameInfo.commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_pipelineLayout,
            0,
            static_cast<uint32_t>(descriptorSets.size()),
            descriptorSets.data(),
            0,
            nullptr
        );

        auto view = frameInfo.scene.getEntitiesWith<TransformComponent, MeshComponent, MaterialComponent>();
        for (auto entity : view) {

            const auto&[transform, meshComponent, materialComponent] = view.get<TransformComponent, MeshComponent, MaterialComponent>(entity);

			auto material = materialComponent.material;
            auto vulkanMesh = std::static_pointer_cast<VulkanMesh>(meshComponent.mesh);

            MaterialPushConstantData push{};
            push.modelMatrix = transform.mat4();
            push.normalMatrix = transform.normalMatrix();
            push.color = material->getAlbedoColor() * glm::vec4(materialComponent.tint, 1.0f);
            push.specularIntensity = 0.0f;
            push.shininess = 1.0f;
            push.textureIndex = m_textureRegistry.getIndex(material->getAlbedoMap()->id);
            push.normalMapIndex = m_textureRegistry.getIndex(material->getNormalMap()->id);
			//push.metallicMapIndex = m_textureRegistry.getIndex(material->getMetallicMap()->id);
			//push.roughnessMapIndex = m_textureRegistry.getIndex(material->getRoughnessMap()->id);
            push.ambientOcclusionMapIndex = m_textureRegistry.getIndex(material->getAmbientOcclusionMap()->id);
            push.tilingFactor = materialComponent.tilingFactor;

            vkCmdPushConstants(
                frameInfo.commandBuffer,
                m_pipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(MaterialPushConstantData),
                &push);
            
            vulkanMesh->bind(frameInfo.commandBuffer);
            vulkanMesh->draw(frameInfo.commandBuffer);

        }
    }
}